#include <iostream>
#include <stdexcept>
#include "buffer_config.hpp"

#include <string>
#include <jungfrau.hpp>
#include <thread>
#include <chrono>
#include <FastQueue.hpp>
#include <cstring>
#include <zmq.h>
#include <LiveRecvModule.hpp>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>

using namespace std;
using namespace core_buffer;

int main (int argc, char *argv[])
{
    if (argc != 2) {
        cout << endl;
        cout << "Usage: sf_stream ";
        cout << " [config_json_file]";
        cout << endl;
        cout << "\tconfig_json_file: json file with the configuration parameters(detector name, number of modules, pedestal and gain files" << endl;
        cout << endl;

        exit(-1);
    }

    string config_json_file = string(argv[1]);
  
    ifstream ifs(config_json_file);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document config_parameters;
    config_parameters.ParseStream(isw);

    string streamvis_address = config_parameters["streamvis_stream"].GetString();
    int reduction_factor_streamvis = config_parameters["streamvis_rate"].GetInt();
    string live_analysis_address = config_parameters["live_stream"].GetString();
    int reduction_factor_live_analysis = config_parameters["live_rate"].GetInt();

    const string PEDE_FILENAME = config_parameters["pedestal_file"].GetString();
    const string GAIN_FILENAME = config_parameters["gain_file"].GetString();
    const string DETECTOR_NAME = config_parameters["detector_name"].GetString();
    size_t n_modules = config_parameters["n_modules"].GetInt();

    FastQueue<ModuleFrameBuffer> queue(
            n_modules * MODULE_N_BYTES,
            STREAM_FASTQUEUE_SLOTS);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);

    const string LIVE_IPC_URL = BUFFER_LIVE_IPC_URL+DETECTOR_NAME+"-";

    LiveRecvModule recv_module(queue, n_modules, ctx, LIVE_IPC_URL);

    // 0mq sockets to streamvis and live analysis
    void *socket_streamvis = zmq_socket(ctx, ZMQ_PUB);
    if (zmq_bind(socket_streamvis, streamvis_address.c_str()) != 0) {
        throw runtime_error(strerror(errno));
    }
    void *socket_live = zmq_socket(ctx, ZMQ_PUB);
    if (zmq_bind(socket_live, live_analysis_address.c_str()) != 0) {
        throw runtime_error(strerror(errno));
    }

    uint16_t data_empty [] = { 0, 0, 0, 0};



    // TODO: Remove stats trash.
    int stats_counter = 0;

    size_t read_total_us = 0;
    size_t read_max_us = 0;

    while (true) {

        rapidjson::Document header(rapidjson::kObjectType);
        auto& header_alloc = header.GetAllocator();
        string text_header;

        auto start_time = chrono::steady_clock::now();

        auto slot_id = queue.read();

        if(slot_id == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    core_buffer::RB_READ_RETRY_INTERVAL_MS));
            continue;
        }

        auto metadata = queue.get_metadata_buffer(slot_id);
        auto data = queue.get_data_buffer(slot_id);

        auto read_end_time = chrono::steady_clock::now();
        auto read_us_duration = chrono::duration_cast<chrono::microseconds>(
                read_end_time-start_time).count();

        uint64_t pulse_id    = 0;
        uint64_t frame_index = 0;
        uint64_t daq_rec     = 0;
        bool is_good_frame   = true;

        for (size_t i_module = 0; i_module < n_modules; i_module++) {
        // TODO: Place this tests in the appropriate spot.
            auto& module_metadata = metadata->module[i_module];
            if (i_module == 0) {
                pulse_id    = module_metadata.pulse_id;
                frame_index = module_metadata.frame_index;
                daq_rec     = module_metadata.daq_rec;

                if ( module_metadata.n_received_packets != 128 ) is_good_frame = false;
            } else {
                if (module_metadata.pulse_id != pulse_id) is_good_frame = false;

                if (module_metadata.frame_index != frame_index) is_good_frame = false;

                if (module_metadata.daq_rec != daq_rec) is_good_frame = false;

                if (module_metadata.n_received_packets != 128 ) is_good_frame = false;
            }
        }

        // TODO: Here we need to send to streamvis and live analysis metadata(probably need to operate still on them) and data(not every frame)

        header.AddMember("frame", frame_index, header_alloc);
        header.AddMember("is_good_frame", is_good_frame, header_alloc);
        header.AddMember("daq_rec", daq_rec, header_alloc);
        header.AddMember("pulse_id", pulse_id, header_alloc);

        rapidjson::Value pedestal_file;
        pedestal_file.SetString(PEDE_FILENAME.c_str(), header_alloc);
        header.AddMember("pedestal_file", pedestal_file, header_alloc);

        rapidjson::Value gain_file;
        gain_file.SetString(GAIN_FILENAME.c_str(), header_alloc);
        header.AddMember("gain_file", gain_file, header_alloc);

        header.AddMember("number_frames_expected", 10000, header_alloc);

        rapidjson::Value run_name;
        run_name.SetString(
                to_string(uint64_t(pulse_id/10000)*10000).c_str(),
                header_alloc);
        header.AddMember("run_name", run_name, header_alloc);

        rapidjson::Value detector_name;
        detector_name.SetString(DETECTOR_NAME.c_str(), header_alloc);
        header.AddMember("detector_name", detector_name, header_alloc);

        header.AddMember("htype", "array-1.0", header_alloc);
        header.AddMember("type", "uint16", header_alloc);

        // To be retrieved and filled with correct values down.
        auto shape_value = rapidjson::Value(rapidjson::kArrayType);
        shape_value.PushBack((uint64_t)0, header_alloc);
        shape_value.PushBack((uint64_t)0, header_alloc);
        header.AddMember("shape", shape_value, header_alloc);

        int send_streamvis = 0;
        if ( reduction_factor_streamvis > 1 ) {
            send_streamvis = rand() % reduction_factor_streamvis;
        }
        if ( send_streamvis == 0 ) {
            auto& shape = header["shape"];
            shape[0] = 16384;
            shape[1] = 1024;
        } else{
            auto& shape = header["shape"];
            shape[0] = 2;
            shape[1] = 2;
        }

        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            header.Accept(writer);

            text_header = buffer.GetString();
        }

        zmq_send(socket_streamvis,
             text_header.c_str(),
             text_header.size(),
             ZMQ_SNDMORE);

        if ( send_streamvis == 0 ) {
            zmq_send(socket_streamvis,
            (char*)data,
            core_buffer::MODULE_N_BYTES*n_modules,
            0);
        } else {
            zmq_send(socket_streamvis,
            (char*)data_empty,
            8,
            0);
        }

        //same for live analysis
        int send_live_analysis = 0;
        if ( reduction_factor_live_analysis > 1 ) {
            send_live_analysis = rand() % reduction_factor_live_analysis;
        }
        if ( send_live_analysis == 0 ) {
            auto& shape = header["shape"];
            shape[0] = 16384;
            shape[1] = 1024;
        } else{
            auto& shape = header["shape"];
            shape[0] = 2;
            shape[1] = 2;
        }

        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            header.Accept(writer);

            text_header = buffer.GetString();
        }

        zmq_send(socket_live,
             text_header.c_str(),
             text_header.size(),
             ZMQ_SNDMORE);

        if ( send_live_analysis == 0 ) {
            zmq_send(socket_live,
            (char*)data,
            core_buffer::MODULE_N_BYTES*n_modules,
            0);
        } else {
            zmq_send(socket_live,
            (char*)data_empty,
            8,
            0);
        }

        queue.release();

        // TODO: Some poor statistics.
        stats_counter++;
        read_total_us += read_us_duration;

        if (read_us_duration > read_max_us) {
            read_max_us = read_us_duration;
        }

        if (stats_counter == STATS_MODULO) {
            cout << "sf_stream:read_us " << read_total_us / STATS_MODULO;
            cout << " sf_stream:read_max_us " << read_max_us;
            cout << endl;

            stats_counter = 0;
            read_total_us = 0;
            read_max_us = 0;
        }
    }

    return 0;
}
