#include <iostream>
#include <stdexcept>
#include "buffer_config.hpp"
#include "zmq.h"
#include <string>
#include <jungfrau.hpp>
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
        WriterZmqReceiver receiver(ctx, ipc_prefix, n_modules);

        uint64_t current_pulse_id=start_pulse_id;

        // "<= stop_pulse_id" because we include the last pulse_id.
        while(current_pulse_id<=stop_pulse_id) {

            int slot_id;
            while((slot_id = queue.reserve()) == -1) {
                this_thread::sleep_for(chrono::milliseconds(
                        RB_READ_RETRY_INTERVAL_MS));
            }

            auto metadata = queue.get_metadata_buffer(slot_id);
            auto buffer = queue.get_data_buffer(slot_id);

            receiver.get_next_buffer(
                    current_pulse_id, metadata, buffer);

            queue.commit();
            current_pulse_id += metadata->n_images;
        }

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
    thread replay_receive_thread(receive_replay,
                                 ctx, ipc_base, n_modules,
                                 ref(queue), start_pulse_id, stop_pulse_id);

    size_t n_frames = stop_pulse_id - start_pulse_id + 1;
    WriterH5Writer writer(output_file, n_frames, n_modules);

    auto current_pulse_id = start_pulse_id;
    // "<= stop_pulse_id" because we include the last pulse_id.
    while (current_pulse_id <= stop_pulse_id) {

        auto start_time = chrono::steady_clock::now();

        int slot_id;
        while((slot_id = queue.read()) == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    RB_READ_RETRY_INTERVAL_MS));
        }

        auto metadata = queue.get_metadata_buffer(slot_id);
        auto data = queue.get_data_buffer(slot_id);

        // Verify that all pulse_ids are correct.
        for (int i=0; i<metadata->n_images; i++) {
            if (metadata->pulse_id[i] != current_pulse_id) {
                throw runtime_error("Wrong pulse id from receiver thread.");
            }

            current_pulse_id++;
        }

        auto end_time = chrono::steady_clock::now();
        auto read_us_duration = chrono::duration_cast<chrono::microseconds>(
                end_time-start_time).count();

        start_time = chrono::steady_clock::now();

        writer.write(metadata, data);

        end_time = chrono::steady_clock::now();
        auto write_us_duration = chrono::duration_cast<chrono::microseconds>(
                end_time-start_time).count();

        queue.release();

        auto avg_read_us = read_us_duration / metadata->n_images;;
        auto avg_write_us = write_us_duration / metadata->n_images;;

        cout << "sf_writer:avg_read_us " << avg_read_us;
        cout << " sf_writer:avg_write_us " << avg_write_us;
        cout << endl;
    }

    writer.close_file();

    //wait till receive thread is finished
    replay_receive_thread.join(); 
    return 0;
}
