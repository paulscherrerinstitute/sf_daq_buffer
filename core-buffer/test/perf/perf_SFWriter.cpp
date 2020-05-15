#include <iostream>
#include "buffer_config.hpp"
#include "zmq.h"
#include <string>
#include <RingBuffer.hpp>
#include <thread>
#include <chrono>
#include "WriterH5Writer.hpp"

using namespace std;
using namespace core_buffer;


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

    size_t n_frames = stop_pulse_id - start_pulse_id;
    WriterH5Writer writer(output_file, n_frames, n_modules);

    // TODO: Remove stats trash.
    int i_write = 0;
    size_t total_ms = 0;
    size_t max_ms = 0;
    size_t min_ms = 10000; // 10 seconds should be a safe first value.

    auto start_time = chrono::steady_clock::now();

    auto metadata = make_shared<DetectorFrame>();
    auto data = make_unique<char[]>(MODULE_N_BYTES*n_modules);

    auto current_pulse_id = start_pulse_id;
    while (current_pulse_id <= stop_pulse_id) {

        writer.write(metadata.get(), data.get());
        current_pulse_id++;

        i_write++;

        auto end_time = chrono::steady_clock::now();

        // TODO: Some poor statistics.

        auto ms_duration = chrono::duration_cast<chrono::milliseconds>(
                end_time-start_time).count();
        total_ms += ms_duration;

        if (ms_duration > max_ms) {
            max_ms = ms_duration;
        }

        if (ms_duration < min_ms) {
            min_ms = ms_duration;
        }

        if (i_write==100) {
            cout << "avg_write_ms " << total_ms / 100;
            cout << " min_write_ms " << min_ms;
            cout << " max_write_ms " << max_ms << endl;

            i_write = 0;
            total_ms = 0;
            max_ms = 0;
            min_ms = 0;
        }

        start_time = chrono::steady_clock::now();
    }

    writer.close_file();

    return 0;
}
