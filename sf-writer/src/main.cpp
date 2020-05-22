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
#include <BufferedFastQueue.hpp>
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
        BufferedFastQueue buffered_queue(
                queue, WRITER_DATA_CACHE_N_IMAGES, n_modules);

        uint64_t current_pulse_id=start_pulse_id;

        // "<= stop_pulse_id" because we include the last pulse_id.
        while(current_pulse_id<=stop_pulse_id) {

            auto image_metadata = buffered_queue.get_metadata_buffer();
            auto image_buffer = buffered_queue.get_data_buffer();

            receiver.get_next_image(
                    current_pulse_id, image_metadata, image_buffer);

            if (image_metadata->pulse_id != current_pulse_id) {
                throw runtime_error("Wrong pulse id from zmq receiver.");
            }

            buffered_queue.commit();
            current_pulse_id++;
        }

        buffered_queue.finalize();

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

    // TODO: Remove stats trash.
    int stats_counter = 0;
    size_t read_total_us = 0;
    size_t write_total_us = 0;
    size_t read_max_us = 0;
    size_t write_max_us = 0;

    auto start_time = chrono::steady_clock::now();

    auto current_pulse_id = start_pulse_id;
    // "<= stop_pulse_id" because we include the last pulse_id.
    while (current_pulse_id <= stop_pulse_id) {

        int slot_id; ;
        while((slot_id = queue.read()) == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    RB_READ_RETRY_INTERVAL_MS));
        }

        auto metadata = queue.get_metadata_buffer(slot_id);
        auto data = queue.get_data_buffer(slot_id);

        auto read_end_time = chrono::steady_clock::now();
        auto read_us_duration = chrono::duration_cast<chrono::microseconds>(
                read_end_time-start_time).count();

        // Verify that all pulse_ids are correct.
        for (int i=0; i<metadata->n_pulses_in_buffer; i++) {
            if (metadata->pulse_id[i] != current_pulse_id) {
                throw runtime_error("Wrong pulse id from receiver thread.");
            }

            current_pulse_id++;
        }

        start_time = chrono::steady_clock::now();

        writer.write(metadata, data);

        auto write_end_time = chrono::steady_clock::now();
        auto write_us_duration = chrono::duration_cast<chrono::microseconds>(
                write_end_time-start_time).count();

        queue.release();

        // TODO: Some poor statistics.
        stats_counter++;

        read_total_us += read_us_duration;
        read_max_us = max(read_max_us, (uint64_t)read_us_duration);

        write_total_us += write_us_duration;
        write_max_us = max(write_max_us, (uint64_t)write_us_duration);

//        if (stats_counter == STATS_MODULO) {
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
//        }

        start_time = chrono::steady_clock::now();
    }

    writer.close_file();

    //wait till receive thread is finished
    replay_receive_thread.join(); 
    return 0;
}
