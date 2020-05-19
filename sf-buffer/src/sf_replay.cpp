#include <iostream>
#include <thread>
#include "jungfrau.hpp"

#include "zmq.h"
#include "buffer_config.hpp"

#include <cstring>
#include <ReplayH5Reader.hpp>
#include "date.h"
#include "bitshuffle/bitshuffle.h"

using namespace std;
using namespace core_buffer;

void sf_replay (
        void* socket,
        const string& device,
        const string& channel_name,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id)
{
    StreamModuleFrame metadata_buffer;
    auto frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);

//    auto compressed_buffer_size = bshuf_compress_lz4_bound(
//            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
//    auto compressed_buffer = make_unique<char[]>(compressed_buffer_size);

    ReplayH5Reader file_reader(device, channel_name);

    //TODO: Add statstics.
    uint64_t stats_counter = 0;

    uint64_t total_read_us = 0;
    uint64_t max_read_us = 0;
    uint64_t total_compress_us = 0;
    uint64_t max_compress_us = 0;
    uint64_t total_send_us = 0;
    uint64_t max_send_us = 0;

    uint64_t total_original_size = 0;
    uint64_t total_compressed_size = 0;

    // "<= stop_pulse_id" because we include the stop_pulse_id in the file.
    for (
            uint64_t curr_pulse_id = start_pulse_id;
            curr_pulse_id <= stop_pulse_id;
            curr_pulse_id++) {

        auto start_time = chrono::steady_clock::now();

        metadata_buffer.is_frame_present = file_reader.get_frame(
                curr_pulse_id,
                &(metadata_buffer.metadata),
                (char*)(frame_buffer.get()));

        metadata_buffer.data_n_bytes = MODULE_N_BYTES;

        auto end_time = chrono::steady_clock::now();
        auto read_us_duration = chrono::duration_cast<chrono::microseconds>(
                end_time-start_time).count();

//        start_time = chrono::steady_clock::now();
//
//        auto compressed_size = bshuf_compress_lz4(
//                frame_buffer.get(), compressed_buffer.get(),
//                MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
//
//        if (compressed_size < 0) {
//            throw runtime_error("Error while compressing buffer.");
//        }
//
//        metadata_buffer.frame_data_n_bytes = compressed_size;
//
//        end_time = chrono::steady_clock::now();
//        auto compress_us_duration = chrono::duration_cast<chrono::microseconds>(
//                end_time-start_time).count();

        start_time = chrono::steady_clock::now();

        zmq_send(socket,
                 &metadata_buffer,
                 sizeof(StreamModuleFrame),
                 ZMQ_SNDMORE);
        zmq_send(socket,
                 (char*)(frame_buffer.get()),
                 metadata_buffer.data_n_bytes,
                 0);

        end_time = chrono::steady_clock::now();
        auto send_us_duration = chrono::duration_cast<chrono::microseconds>(
                end_time-start_time).count();

        // TODO: Make proper stastistics.
        stats_counter++;
        total_read_us += read_us_duration;
        max_read_us = max(max_read_us, (uint64_t)read_us_duration);

//        total_compress_us += compress_us_duration;
//        max_compress_us = max(max_compress_us, (uint64_t)compress_us_duration);

        total_send_us += send_us_duration;
        max_send_us = max(max_send_us, (uint64_t)send_us_duration);

        total_compressed_size += metadata_buffer.data_n_bytes;
        total_original_size += MODULE_N_BYTES + sizeof(StreamModuleFrame);

        if (stats_counter == STATS_MODULO) {
            cout << "sf_replay:avg_read_us " << total_read_us/STATS_MODULO;
            cout << " sf_replay:max_read_us " << max_read_us;

            cout << " sf_replay:avg_compress_us ";
            cout << total_compress_us/STATS_MODULO;
            cout << " sf_replay:max_compress_us " << max_compress_us;

            cout << " sf_replay:avg_send_us " << total_send_us/STATS_MODULO;
            cout << " sf_replay:max_send_us " << max_send_us;

            cout << " sf_replay:compress_ratio ";
            cout << (float)total_compressed_size/total_original_size;
            cout << endl;

            stats_counter = 0;
            total_read_us = 0;
            max_read_us = 0;
            total_compress_us = 0;
            max_compress_us = 0;
            total_send_us = 0;
            max_send_us = 0;
            total_original_size = 0;
            total_compressed_size = 0;
        }
    }
}

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_replay [device]";
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

    const string device = string(argv[1]);
    const string channel_name = string(argv[2]);
    const auto source_id = (uint16_t) atoi(argv[3]);
    const auto start_pulse_id = (uint64_t) atoll(argv[4]);
    const auto stop_pulse_id = (uint64_t) atoll(argv[5]);

    stringstream ipc_stream;
    ipc_stream << REPLAY_STREAM_IPC_URL << (int)source_id;
    const auto ipc_address = ipc_stream.str();

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUSH);

    const int sndhwm = REPLAY_SNDHWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(strerror (errno));

    const int linger_ms = -1;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(strerror (errno));

    if (zmq_bind(socket, ipc_address.c_str()) != 0)
        throw runtime_error(strerror (errno));

    sf_replay(socket, device, channel_name, start_pulse_id, stop_pulse_id);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
}
