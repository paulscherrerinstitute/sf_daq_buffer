#include <iostream>
#include <stdexcept>
#include "buffer_config.hpp"
#include "zmq.h"
#include <string>
#include <jungfrau.hpp>
#include <thread>
#include <chrono>
#include "SFWriter.hpp"
#include <FastQueue.hpp>
#include <cstring>
#include "date.h"
#include <jsoncpp/json/json.h>

using namespace std;
using namespace core_buffer;

const int WRITER_N_FRAMES_BUFFER_live = 1;

void receive_replay(
        const string ipc_prefix,
        const size_t n_modules,
        FastQueue<DetectorFrame>& queue,
        void* ctx)
{
    try {

        void *sockets[n_modules];
        for (size_t i = 0; i < n_modules; i++) {
            //sockets[i] = zmq_socket(ctx, ZMQ_PULL);
            sockets[i] = zmq_socket(ctx, ZMQ_SUB);
            //int rcvhwm = REPLAY_READ_BLOCK_SIZE;
            //if (zmq_setsockopt(sockets[i], ZMQ_RCVHWM, &rcvhwm,
            //                   sizeof(rcvhwm)) != 0) {
            //    throw runtime_error(strerror(errno));
            //}
            //int linger = 0;
            //if (zmq_setsockopt(sockets[i], ZMQ_LINGER, &linger,
            //                   sizeof(linger)) != 0) {
            //    throw runtime_error(strerror(errno));
            //}

            stringstream ipc_addr;
            ipc_addr << ipc_prefix << i;
            const auto ipc = ipc_addr.str();

            if (zmq_connect(sockets[i], ipc.c_str()) != 0) {
                throw runtime_error(strerror(errno));
            }
            zmq_setsockopt(sockets[i], ZMQ_SUBSCRIBE, "", sizeof("") != 0);
        }

        auto module_meta_buffer = make_unique<ModuleFrame>();

        while (true) {

            auto slot_id = queue.reserve();

            if (slot_id == -1){
                this_thread::sleep_for(chrono::milliseconds(5));
                continue;
            }

            auto frame_meta_buffer = queue.get_metadata_buffer(slot_id);
            auto frame_buffer = queue.get_data_buffer(slot_id);

            for (
                    size_t i_buffer=0;
                    i_buffer<WRITER_N_FRAMES_BUFFER_live;
                    i_buffer++)
            {

                frame_meta_buffer->is_good_frame[i_buffer] = true;

                for (size_t i_module = 0; i_module < n_modules; i_module++) {
                    if (i_module == 0 ) {
                        cout << "before zmq_recv" << endl;
                    }
                    auto n_bytes_metadata = zmq_recv(
                            sockets[i_module],
                            module_meta_buffer.get(),
                            sizeof(ModuleFrame),
                            0);
                    if (i_module == 0 ) {
                        cout << "after zmq_recv" << endl;
                    }


                    if (n_bytes_metadata != sizeof(ModuleFrame)) {
                        // TODO: Make nicer expcetion.
                        frame_meta_buffer->is_good_frame[i_buffer] = false;
                        throw runtime_error(strerror(errno));
                    }

                    // Initialize buffers in first iteration for each pulse_id.
                    if (i_module == 0) {
                        frame_meta_buffer->pulse_id[i_buffer] =
                                module_meta_buffer->pulse_id;
                        frame_meta_buffer->frame_index[i_buffer] =
                                module_meta_buffer->frame_index;
                        frame_meta_buffer->daq_rec[i_buffer] =
                                module_meta_buffer->daq_rec;
                        frame_meta_buffer->n_received_packets[i_buffer] =
                                module_meta_buffer->n_received_packets;
                    }

                    if (frame_meta_buffer->pulse_id[i_buffer] !=
                        module_meta_buffer->pulse_id) {
                        frame_meta_buffer->is_good_frame[i_buffer] = false;
                        //throw runtime_error("Unexpected pulse_id received.");
                    }

                    if (frame_meta_buffer->frame_index[i_buffer] !=
                        module_meta_buffer->frame_index) {
                        frame_meta_buffer->is_good_frame[i_buffer] = false;
                    }

                    if (frame_meta_buffer->daq_rec[i_buffer] !=
                        module_meta_buffer->daq_rec) {
                        frame_meta_buffer->is_good_frame[i_buffer] = false;
                    }

                    if ( module_meta_buffer->n_received_packets != 128 ) {
                        frame_meta_buffer->is_good_frame[i_buffer] = false;
                    }

                    // Offset due to frame in buffer.
                    size_t offset = MODULE_N_BYTES * n_modules * i_buffer;
                    // offset due to module in frame.
                    offset += MODULE_N_BYTES * i_module;

                    auto n_bytes_image = zmq_recv(
                            sockets[i_module],
                            (frame_buffer + offset),
                            MODULE_N_BYTES,
                            0);

                    if (n_bytes_image != MODULE_N_BYTES) {
                        // TODO: Make nicer expcetion.
                        throw runtime_error("Unexpected number of bytes.");
                    }
                }
            }

            queue.commit();
        }

        for (size_t i = 0; i < n_modules; i++) {
            zmq_close(sockets[i]);
        }

        zmq_ctx_destroy(ctx);
    } catch (const std::exception& e) {
        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[sf_stream::receive_replay]";
        cout << " Stopped because of exception: " << endl;
        cout << e.what() << endl;

        throw;
    }
}

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

    FastQueue<DetectorFrame> queue(
            n_modules * MODULE_N_BYTES * WRITER_N_FRAMES_BUFFER_live,
            WRITER_RB_BUFFER_SLOTS);

    string ipc_prefix = "ipc:///tmp/sf-live-";
    
    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, WRITER_ZMQ_IO_THREADS);

    thread replay_receive_thread(
            receive_replay,
            ipc_prefix,
            n_modules,
            ref(queue),
            ctx);

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
    size_t write_total_us = 0;
    size_t read_max_us = 0;
    size_t write_max_us = 0;

    auto start_time = chrono::steady_clock::now();

    Json::Value header;
    Json::StreamWriterBuilder builder;

    while (true) {

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

        start_time = chrono::steady_clock::now();

        //Here we need to send to streamvis and live analysis metadata(probably need to operate still on them) and data(not every frame)
        
        for ( size_t i_buffer=0; i_buffer<WRITER_N_FRAMES_BUFFER_live; i_buffer++) {
            //for (size_t i_module = 0; i_module < n_modules; i_module++) {
            //    cout << metadata->pulse_id[i_buffer*n_modules+i_module] << " ";
            //}
            //cout << endl;
            //cout << metadata->is_good_frame[i_buffer] << " " <<  metadata->pulse_id[i_buffer] << " " << metadata->frame_index[i_buffer] << " " << metadata->daq_rec[i_buffer] << " " << metadata->n_received_packets[i_buffer] << " " << endl;
            header["frame"] = (Json::Value::UInt64)metadata->frame_index[i_buffer];
            header["is_good_frame"] = metadata->is_good_frame[i_buffer];
            header["daq_rec"] = metadata->daq_rec[i_buffer];
            header["pulse_id"] = (Json::Value::UInt64)metadata->pulse_id[i_buffer];

            //this needs to be re-read from external source
            header["pedestal_file"] = "/sf/bernina/data/p17534/res/JF_pedestals/pedestal_20200423_1018.JF07T32V01.res.h5"; 
            header["gain_file"] = "/sf/bernina/config/jungfrau/gainMaps/JF07T32V01/gains.h5";

            header["number_frames_expected"] = 10000;
            header["run_name"] = to_string(uint64_t(metadata->pulse_id[i_buffer]/10000)*10000);

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


        }

        queue.release();

        // TODO: Some poor statistics.
        stats_counter += WRITER_N_FRAMES_BUFFER_live;
        auto write_end_time = chrono::steady_clock::now();
        auto write_us_duration = chrono::duration_cast<chrono::microseconds>(
                write_end_time-start_time).count();

        read_total_us += read_us_duration;
        write_total_us += write_us_duration;

        if (read_us_duration > read_max_us) {
            read_max_us = read_us_duration;
        }

        if (write_us_duration > write_max_us) {
            write_max_us = write_us_duration;
        }

        if (stats_counter == STATS_MODULO) {
            cout << "sf_stream:read_us " << read_total_us / STATS_MODULO;
            cout << " sf_stream:read_max_us " << read_max_us;
            cout << " sf_stream:write_us " << write_total_us / STATS_MODULO;
            cout << " sf_stream:write_max_us " << write_max_us;

            cout << endl;

            stats_counter = 0;
            read_total_us = 0;
            read_max_us = 0;
            write_total_us = 0;
            write_max_us = 0;
        }

        start_time = chrono::steady_clock::now();
    }

    return 0;
}
