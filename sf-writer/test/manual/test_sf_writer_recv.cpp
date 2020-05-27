#include <iostream>
#include "buffer_config.hpp"
#include "zmq.h"
#include <string>
#include <thread>
#include <chrono>
#include "JFH5Writer.hpp"
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
        FastQueue<ImageMetadataBlock>& queue,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id)
{
    try {
        WriterZmqReceiver receiver(ctx, ipc_prefix, n_modules, stop_pulse_id);

        int slot_id;
        while((slot_id = queue.reserve()) == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    RB_READ_RETRY_INTERVAL_MS));
        }

        uint64_t pulse_id = start_pulse_id;
        // "<= stop_pulse_id" because we include the last pulse_id.
        while(pulse_id <= stop_pulse_id) {
            auto start_time = chrono::steady_clock::now();

            auto image_metadata = queue.get_metadata_buffer(slot_id);
            auto image_buffer = queue.get_data_buffer(slot_id);

            receiver.get_next_buffer(pulse_id, image_metadata, image_buffer);

            pulse_id += image_metadata->n_images;

            auto end_time = chrono::steady_clock::now();
            auto read_us_duration = chrono::duration_cast<chrono::microseconds>(
                    end_time-start_time).count();

            cout << "sf_writer::avg_read_us ";
            cout << read_us_duration / image_metadata->n_images << endl;
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

    FastQueue<ImageMetadataBlock> queue(
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
