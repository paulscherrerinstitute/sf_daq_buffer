#include <iostream>
#include <thread>
#include <FastQueue.hpp>
#include <cstring>

#include "buffer_config.hpp"
#include "ReplayH5Reader.hpp"
#include "ReplayZmqSender.hpp"

using namespace std;
using namespace core_buffer;
using namespace chrono;

void sf_replay (
        const string device,
        const string channel_name,
        FastQueue<ModuleFrame>& queue,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id
        )
{
    uint64_t read_us = 0;
    uint64_t max_read_us = 0;
    uint64_t n_stats = 0;

    ReplayH5Reader file_reader(device, channel_name);

    // "<= stop_pulse_id" because we include the stop_pulse_id in the file.
    for (uint64_t curr_pulse_id=start_pulse_id;
         curr_pulse_id <= stop_pulse_id;
         curr_pulse_id++) {

        int slot_id;
        while((slot_id = queue.reserve()) == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    RB_READ_RETRY_INTERVAL_MS));
        }

        auto start_time = steady_clock::now();

        auto metadata = queue.get_metadata_buffer(slot_id);
        auto buffer = queue.get_data_buffer(slot_id);

        ModuleFrame* m_buffer;
        char* f_buffer;
        file_reader.get_buffer(curr_pulse_id, m_buffer, f_buffer);

        memcpy(metadata, m_buffer, sizeof(ModuleFrame));
        memcpy(buffer, f_buffer, MODULE_N_BYTES);

        auto end_time = steady_clock::now();
        uint64_t read_us_duration =
                duration_cast<microseconds>(end_time-start_time).count();

        queue.commit();

        // TODO: Proper statistics
        n_stats++;

        read_us += read_us_duration;
        max_read_us = max(max_read_us, read_us_duration);

        if (n_stats == STATS_MODULO) {
            cout << "sf_replay:avg_read_us " << read_us / STATS_MODULO;
            cout << " sf_replay:max_read_us " << max_read_us;
            cout << endl;

            n_stats = 0;
            read_us = 0;
            max_read_us = 0;
        }
    }
}

int main (int argc, char *argv[]) {

    if (argc != 7) {
        cout << endl;
        cout << "Usage: sf_replay [ipc_id] [device]";
        cout << " [channel_name] [source_id] [start_pulse_id] [stop_pulse_id]";
        cout << endl;
        cout << "\tdevice: Name of detector." << endl;
        cout << "\tchannel_name: M00-M31 for JF16M." << endl;
        cout << "\tsource_id: Module index" << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tstop_pulse_id: Stop pulse_id of retrieval." << endl;
        cout << endl;

        exit(-1);
    }

    const string ipc_id = string(argv[1]);
    const string device = string(argv[2]);
    const string channel_name = string(argv[3]);
    const auto source_id = atoi(argv[4]);
    const auto start_pulse_id = (uint64_t) atoll(argv[5]);
    const auto stop_pulse_id = (uint64_t) atoll(argv[6]);

    FastQueue<ModuleFrame> queue(MODULE_N_BYTES, REPLAY_FASTQUEUE_N_SLOTS);

    thread file_read_thread(sf_replay,
                            device, channel_name, ref(queue),
                            start_pulse_id, stop_pulse_id);

    uint64_t send_us = 0;
    uint64_t max_send_us = 0;
    uint64_t n_stats = 0;

    ReplayZmqSender sender(ipc_id, source_id);

    // "<= stop_pulse_id" because we include the stop_pulse_id in the file.
    for (uint64_t curr_pulse_id=start_pulse_id;
         curr_pulse_id <= stop_pulse_id;
         curr_pulse_id++) {

        int slot_id;
        while((slot_id = queue.read()) == -1) {
            this_thread::sleep_for(chrono::milliseconds(
                    RB_READ_RETRY_INTERVAL_MS));
        }

        auto m_buffer = queue.get_metadata_buffer(slot_id);
        auto f_buffer = queue.get_data_buffer(slot_id);

        auto start_time = steady_clock::now();

        sender.send(m_buffer, f_buffer);

        auto end_time = steady_clock::now();
        uint64_t send_us_duration =
                duration_cast<microseconds>(end_time-start_time).count();

        queue.release();

        // TODO: Proper statistics
        n_stats++;

        send_us += send_us_duration;
        max_send_us = max(max_send_us, send_us_duration);

        if (n_stats == STATS_MODULO) {
            cout << "sf_replay:avg_send_us " << send_us / STATS_MODULO;
            cout << " sf_replay:max_send_us " << max_send_us;
            cout << endl;

            n_stats = 0;
            send_us = 0;
            max_send_us = 0;
        }
    }

    file_read_thread.join();
    return 0;
}
