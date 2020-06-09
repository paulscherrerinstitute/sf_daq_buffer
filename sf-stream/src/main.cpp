#include <iostream>
#include <string>
#include <chrono>
#include <cstring>
#include <zmq.h>

#include "FastQueue.hpp"
#include "LiveRecvModule.hpp"
#include "buffer_config.hpp"
#include "stream_config.hpp"
#include "ZmqLiveSender.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace stream_config;

int main (int argc, char *argv[])
{
    if (argc != 2) {
        cout << endl;
        cout << "Usage: sf_stream ";
        cout << " [config_json_file]";
        cout << endl;
        cout << "\tconfig_json_file: json file with the configuration "
                "parameters(detector name, number of modules, pedestal and "
                "gain files" << endl;
        cout << endl;

        exit(-1);
    }

    auto config = read_json_config(string(argv[1]));
    string RECV_IPC_URL = BUFFER_LIVE_IPC_URL + config.DETECTOR_NAME + "-";

    FastQueue<ModuleFrameBuffer> queue(
            config.n_modules * MODULE_N_BYTES, STREAM_FASTQUEUE_SLOTS);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);

    ZmqLiveReceiver receiver(config.n_modules, ctx, RECV_IPC_URL);
    LiveRecvModule recv_module(queue, receiver);

    ZmqLiveSender sender(ctx, config);

    // TODO: Remove stats trash.
    int stats_counter = 0;
    size_t read_total_us = 0;
    size_t read_max_us = 0;
    size_t send_total_us = 0;
    size_t send_max_us = 0;

    while (true) {

        auto start_time = steady_clock::now();

        int slot_id;
        while((slot_id = queue.read()) == -1) {
            this_thread::sleep_for(milliseconds(RB_READ_RETRY_INTERVAL_MS));
        }

        auto meta = queue.get_metadata_buffer(slot_id);
        auto data = queue.get_data_buffer(slot_id);

        auto end_time = steady_clock::now();
        size_t read_us_duration = duration_cast<microseconds>(
                end_time - start_time).count();

        start_time = steady_clock::now();

        sender.send(meta, data);

        end_time = steady_clock::now();
        size_t send_us_duration = duration_cast<microseconds>(
                end_time - start_time).count();

        queue.release();

        // TODO: Some poor statistics.
        stats_counter++;
        read_total_us += read_us_duration;
        send_total_us += send_us_duration;

        read_max_us = max(read_max_us, read_us_duration);
        send_max_us = max(send_max_us, send_us_duration);

        if (stats_counter == STATS_MODULO) {
            cout << "sf_stream:read_us " << read_total_us / STATS_MODULO;
            cout << " sf_stream:read_max_us " << read_max_us;
            cout << " sf_stream:send_us " << send_total_us / STATS_MODULO;
            cout << " sf_stream:send_max_us " << send_max_us;
            cout << endl;

            stats_counter = 0;
            read_total_us = 0;
            read_max_us = 0;
            send_total_us = 0;
            send_max_us = 0;
        }
    }
}
