#include <iostream>
#include "buffer_config.hpp"
#include "zmq.h"
#include <string>
#include <thread>
#include <chrono>
#include "WriterH5Writer.hpp"
#include <FastQueue.hpp>
#include <cstring>
#include "date.h"
#include "bitshuffle/bitshuffle.h"
#include "WriterZmqReceiver.hpp"

using namespace std;
using namespace core_buffer;

void receive_replay(
        void* ctx,
        const string ipc_prefix,
        const size_t n_modules,
        FastQueue<ImageMetadataBuffer>& queue,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id)
{
    try {

        void* sockets_[n_modules];
        ModuleFrame f_meta_;
        char* image_buffer = new char[MODULE_N_BYTES*n_modules];

        for (size_t i = 0; i < n_modules; i++) {
            sockets_[i] = zmq_socket(ctx, ZMQ_PULL);

            int rcvhwm = WRITER_RCVHWM;
            if (zmq_setsockopt(sockets_[i], ZMQ_RCVHWM, &rcvhwm,
                               sizeof(rcvhwm)) != 0) {
                throw runtime_error(zmq_strerror(errno));
            }
            int linger = 0;
            if (zmq_setsockopt(sockets_[i], ZMQ_LINGER, &linger,
                               sizeof(linger)) != 0) {
                throw runtime_error(zmq_strerror(errno));
            }

            stringstream ipc_addr;
            ipc_addr << ipc_prefix << i;
            const auto ipc = ipc_addr.str();

            if (zmq_connect(sockets_[i], ipc.c_str()) != 0) {
                throw runtime_error(zmq_strerror(errno));
            }
        }

        uint64_t pulse_id = start_pulse_id;
        // "<= stop_pulse_id" because we include the last pulse_id.
        while(pulse_id <= stop_pulse_id) {

            for (size_t i_module = 0; i_module < n_modules; i_module++) {

                auto n_bytes_metadata = zmq_recv(
                        sockets_[i_module], &f_meta_, sizeof(f_meta_), 0);

                if (n_bytes_metadata != sizeof(f_meta_)) {
                    throw runtime_error("Wrong number of metadata bytes.");
                }

                auto module_offset = i_module * MODULE_N_BYTES;

                auto n_bytes_image = zmq_recv(
                        sockets_[i_module],
                        (image_buffer + module_offset),
                        MODULE_N_BYTES, 0);

                if (n_bytes_image != MODULE_N_BYTES) {
                    throw runtime_error("Wrong number of data bytes.");
                }

            }
        }

        queue.commit();

    } catch (const std::exception& e) {
        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[sf_writer::receive_replay]";
        cout << " Stopped because of exception: " << endl;
        cout << e.what() << endl;

        throw;
    }
}

int main (int argc, char *argv[])
{
    if (argc != 5) {
        cout << endl;
        cout << "Usage: sf_writer ";
        cout << " [ipc_id] [output_file] [start_pulse_id] [stop_pulse_id]";
        cout << endl;
        cout << "\tipc_id: Unique identifier for ipc." << endl;
        cout << "\toutput_file: Complete path to the output file." << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tstop_pulse_id: Stop pulse_id of retrieval." << endl;
        cout << endl;

        exit(-1);
    }

    const string ipc_id = string(argv[1]);
    string output_file = string(argv[2]);
    uint64_t start_pulse_id = (uint64_t) atoll(argv[3]);
    uint64_t stop_pulse_id = (uint64_t) atoll(argv[4]);

    size_t n_modules = 32;

    FastQueue<ImageMetadataBuffer> queue(
            MODULE_N_BYTES * n_modules * WRITER_DATA_CACHE_N_IMAGES,
            WRITER_FASTQUEUE_N_SLOTS);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, WRITER_ZMQ_IO_THREADS);

    auto ipc_base = REPLAY_STREAM_IPC_URL + ipc_id + "-";

    receive_replay(
            ctx, ipc_base, n_modules,queue, start_pulse_id, stop_pulse_id);

//
//
//    auto current_pulse_id = start_pulse_id;
//    // "<= stop_pulse_id" because we include the last pulse_id.
//    while (current_pulse_id <= stop_pulse_id) {
//
//        int slot_id;
//        while((slot_id = queue.read()) == -1) {
//            this_thread::sleep_for(chrono::milliseconds(
//                    RB_READ_RETRY_INTERVAL_MS));
//        }
//
//        auto metadata = queue.get_metadata_buffer(slot_id);
//
//        for (int i_pulse=0; i_pulse < metadata->n_images; i_pulse++) {
//            cout << "Written image " << metadata->pulse_id[i_pulse] << endl;
//
//        }
//
//        queue.release();
//        current_pulse_id += metadata->n_images;
//    }
//
//    //wait till receive thread is finished
//    replay_receive_thread.join();
    return 0;
}
