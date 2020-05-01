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

using namespace std;
using namespace core_buffer;

void receive_replay(
        const string ipc_prefix,
        const size_t n_modules,
        FastQueue<DetectorFrame>& queue,
        void* ctx)
{
    try {

        void *sockets[n_modules];
        for (size_t i = 0; i < n_modules; i++) {
            sockets[i] = zmq_socket(ctx, ZMQ_PULL);
            int rcvhwm = REPLAY_READ_BLOCK_SIZE;
            if (zmq_setsockopt(sockets[i], ZMQ_RCVHWM, &rcvhwm,
                               sizeof(rcvhwm)) != 0) {
                throw runtime_error(strerror(errno));
            }
            int linger = 0;
            if (zmq_setsockopt(sockets[i], ZMQ_LINGER, &linger,
                               sizeof(linger)) != 0) {
                throw runtime_error(strerror(errno));
            }

            stringstream ipc_addr;
            ipc_addr << ipc_prefix << i;
            const auto ipc = ipc_addr.str();

            if (zmq_bind(sockets[i], ipc.c_str()) != 0) {
                throw runtime_error(strerror(errno));
            }
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
                    i_buffer<WRITER_N_FRAMES_BUFFER;
                    i_buffer++)
            {

                for (size_t i_module = 0; i_module < n_modules; i_module++) {
                    auto n_bytes_metadata = zmq_recv(
                            sockets[i_module],
                            module_meta_buffer.get(),
                            sizeof(ModuleFrame),
                            0);

                    if (n_bytes_metadata != sizeof(ModuleFrame)) {
                        // TODO: Make nicer expcetion.
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
                        throw runtime_error("Unexpected pulse_id received.");
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
        cout << "[sf_h5_writer::receive_replay]";
        cout << " Stopped because of exception: " << endl;
        cout << e.what() << endl;

        throw;
    }
}

int main (int argc, char *argv[])
{
    if (argc != 4) {
        cout << endl;
        cout << "Usage: sf_writer ";
        cout << " [output_file] [start_pulse_id] [stop_pulse_id]";
        cout << endl;
        cout << "\toutput_file: Complete path to the output file." << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tstop_pulse_id: Stop pulse_id of retrieval." << endl;
        cout << endl;

        exit(-1);
    }

    string output_file = string(argv[1]);
    uint64_t start_pulse_id = (uint64_t) atoll(argv[2]);
    uint64_t stop_pulse_id = (uint64_t) atoll(argv[3]);

    size_t n_modules = 32;

    FastQueue<DetectorFrame> queue(
            n_modules * MODULE_N_BYTES * WRITER_N_FRAMES_BUFFER,
            WRITER_RB_BUFFER_SLOTS);

    string ipc_prefix = "ipc://sf-replay-";
    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, WRITER_ZMQ_IO_THREADS);

    thread replay_receive_thread(
            receive_replay,
            ipc_prefix,
            n_modules,
            ref(queue),
            ctx);

    size_t n_frames = stop_pulse_id - start_pulse_id;
    SFWriter writer(output_file, n_frames, n_modules);

    // TODO: Remove stats trash.
    int stats_counter = 0;

    size_t read_total_us = 0;
    size_t write_total_us = 0;
    size_t read_max_us = 0;
    size_t write_max_us = 0;

    auto start_time = chrono::steady_clock::now();

    auto current_pulse_id = start_pulse_id;
    while (current_pulse_id <= stop_pulse_id) {

        auto slot_id = queue.read();

        if(slot_id == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    RB_READ_RETRY_INTERVAL_MS));
            continue;
        }

        auto metadata = queue.get_metadata_buffer(slot_id);
        auto data = queue.get_data_buffer(slot_id);

        // TODO: This verification should go in the recv part.
//        if (metadata->pulse_id != current_pulse_id) {
//            stringstream err_msg;
//
//            using namespace date;
//            using namespace chrono;
//            err_msg << "[" << system_clock::now() << "]";
//            err_msg << "[sf_writer::main]";
//            err_msg << " Read unexpected pulse_id. ";
//            err_msg << " Expected " << current_pulse_id;
//            err_msg << " received " << metadata->pulse_id;
//            err_msg << endl;
//
//            throw runtime_error(err_msg.str());
//        }

        auto read_end_time = chrono::steady_clock::now();
        auto read_us_duration = chrono::duration_cast<chrono::microseconds>(
                read_end_time-start_time).count();

        start_time = chrono::steady_clock::now();

        writer.write(metadata, data);
        queue.release();
        current_pulse_id += WRITER_N_FRAMES_BUFFER;

        // TODO: Some poor statistics.
        stats_counter += WRITER_N_FRAMES_BUFFER;
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
            cout << "sf_writer:read_us " << read_total_us / STATS_MODULO;
            cout << " sf_writer:read_max_us " << read_max_us;
            cout << " sf_writer:write_us " << write_total_us / STATS_MODULO;
            cout << " sf_writer:write_max_us " << write_max_us;

            cout << endl;

            stats_counter = 0;
            read_total_us = 0;
            read_max_us = 0;
            write_total_us = 0;
            write_max_us = 0;
        }

        start_time = chrono::steady_clock::now();
    }

    writer.close_file();

    return 0;
}
