#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>

#include "date.h"
#include "zmq.h"
#include "jungfrau.hpp"
#include "buffer_config.hpp"
#include "bitshuffle/bitshuffle.h"
#include "WriterZmqReceiver.hpp"
#include "JFH5Writer.hpp"
#include "BufferBinaryReader.hpp"

using namespace std;
using namespace core_buffer;
using namespace chrono;

void read_buffer(
        const string device,
        const string channel_name,
        const vector<uint64_t>& blocks)
{
    BufferBinaryReader block_reader(device, channel_name);



    // "<= stop_block" because we include the stop_block in the transfer.
    for (uint64_t curr_block=start_block;
         curr_block <= stop_block;
         curr_block++) {

        int slot_id;
        while((slot_id = queue.reserve()) == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    RB_READ_RETRY_INTERVAL_MS));
        }

        auto start_time = steady_clock::now();

        auto block_buffer = queue.get_metadata_buffer(slot_id);

        block_reader.get_block(curr_block, block_buffer);

        auto end_time = steady_clock::now();
        uint64_t read_us_duration = duration_cast<microseconds>(
                end_time-start_time).count();

        queue.commit();

        // TODO: Proper statistics
        cout << "sf_replay:avg_read_us ";
        cout << read_us_duration / BUFFER_BLOCK_SIZE << endl;
    }
}

int main (int argc, char *argv[])
{
    if (argc != 5) {
        cout << endl;
        cout << "Usage: sf_writer [output_file] [device]";
        cout << "  [start_pulse_id] [stop_pulse_id]";
        cout << endl;
        cout << "\toutput_file: Complete path to the output file." << endl;
        cout << "\tdevice: Name of detector." << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tstop_pulse_id: Stop pulse_id of retrieval." << endl;
        cout << endl;

        exit(-1);
    }

    string output_file = string(argv[1]);
    const string device = string(argv[2]);
    uint64_t start_pulse_id = (uint64_t) atoll(argv[4]);
    uint64_t stop_pulse_id = (uint64_t) atoll(argv[5]);
    size_t n_modules = 32;

    uint64_t start_block = start_pulse_id / BUFFER_BLOCK_SIZE;
    uint64_t stop_block = stop_pulse_id / BUFFER_BLOCK_SIZE;
    auto n_blocks = stop_block - start_block + 1;

    // Generate list of buffer blocks that need to be loaded.
    std::vector<uint64_t> buffer_blocks(n_blocks);
    for (uint64_t curr_block=start_block;
         curr_block<=stop_block;
         curr_block++) {
        buffer_blocks.push_back(curr_block);
    }

    std::vector<std::thread> reading_threads(n_modules);
    for (size_t i_module=0; i_module<n_modules; i_module++) {

        string channel_name = "M" + to_string(i_module);

        reading_threads.emplace_back(
                read_buffer, device, channel_name, ref(buffer_blocks));
    }

    size_t n_frames = stop_pulse_id - start_pulse_id + 1;
    JFH5Writer writer(output_file, n_frames, n_modules);

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
