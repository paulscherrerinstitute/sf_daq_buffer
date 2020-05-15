#include <iostream>
#include <stdexcept>
#include "buffer_config.hpp"

#include <string>
#include <jungfrau.hpp>
#include <thread>
#include <chrono>
#include "WriterH5Writer.hpp"
#include <FastQueue.hpp>
#include <cstring>
#include <zmq.h>
#include <LiveRecvModule.hpp>
#include "date.h"
#include <jsoncpp/json/json.h>

using namespace std;
using namespace core_buffer;

int main (int argc, char *argv[])
{
    if (argc != 5) {
        cout << endl;
        cout << "Usage: sf_stream ";
        cout << " [streamvis_address] [reduction_factor_streamvis]";
        cout << " [live_analysis_address] [reduction_factor_live_analysis]";
        cout << endl;
        cout << "\tstreamvis_address: address to streamvis, example tcp://129.129.241.42:9007" << endl;
        cout << "\treduction_factor_streamvis: 1 out of N (example 10) images to send to streamvis. For remaining send metadata." << endl;
        cout << "\tlive_analysis_address: address to live_analysis, example tcp://129.129.241.42:9107" << endl;
        cout << "\treduction_factor_live_analysis: 1 out of N (example 10) images to send to live analysis. For remaining send metadata. N<=1 - send every image" << endl;
        cout << endl;

        exit(-1);
    }

    string streamvis_address = string(argv[1]);
    int reduction_factor_streamvis = (int) atoll(argv[2]);
    string live_analysis_address = string(argv[3]);
    int reduction_factor_live_analysis = (uint64_t) atoll(argv[4]);

    size_t n_modules = 32;
    FastQueue<ModuleFrameBuffer> queue(
            n_modules * MODULE_N_BYTES,
            STREAM_FASTQUEUE_SLOTS);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);

    LiveRecvModule recv_module(queue, n_modules, ctx, BUFFER_LIVE_IPC_URL);

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
    Json::Value header;
    Json::StreamWriterBuilder builder;

    // TODO: Remove stats trash.
    int stats_counter = 0;

    size_t read_total_us = 0;
    size_t read_max_us = 0;

    while (true) {

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

        //Here we need to send to streamvis and live analysis metadata(probably need to operate still on them) and data(not every frame)
        
        header["frame"]         = (Json::Value::UInt64)frame_index;
        header["is_good_frame"] = is_good_frame;
        header["daq_rec"]       = (Json::Value::UInt64)daq_rec;
        header["pulse_id"]      = (Json::Value::UInt64)pulse_id;

        //this needs to be re-read from external source
        header["pedestal_file"] = "/sf/bernina/data/p17534/res/JF_pedestals/pedestal_20200423_1018.JF07T32V01.res.h5";
        header["gain_file"] = "/sf/bernina/config/jungfrau/gainMaps/JF07T32V01/gains.h5";

        header["number_frames_expected"] = 10000;
        header["run_name"] = to_string(uint64_t(pulse_id/10000)*10000);

        // detector name should come as parameter to sf_stream
        header["detector_name"] = "JF07T32V01";

        header["htype"] = "array-1.0";
        header["type"]  = "uint16";

        int send_streamvis = 0;
        if ( reduction_factor_streamvis > 1 ) {
            send_streamvis = rand() % reduction_factor_streamvis;
        }
        if ( send_streamvis == 0 ) {
            header["shape"][0] = 16384;
            header["shape"][1] = 1024;
        } else{
            header["shape"][0] = 2;
            header["shape"][1] = 2;
        }

        string text_header = Json::writeString(builder, header);

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
            header["shape"][0] = 16384;
            header["shape"][1] = 1024;
        } else{
            header["shape"][0] = 2;
            header["shape"][1] = 2;
        }

        text_header = Json::writeString(builder, header);

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
