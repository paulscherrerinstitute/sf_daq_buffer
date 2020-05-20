#include <iostream>
#include <thread>
#include "jungfrau.hpp"

#include "zmq.h"
#include "buffer_config.hpp"

#include <cstring>
#include "ReplayH5Reader.hpp"
#include "date.h"
#include "bitshuffle/bitshuffle.h"

using namespace std;
using namespace core_buffer;

void sf_replay (
        void* socket,
        const string& device,
        const string& channel_name,
        const uint16_t source_id,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id)
{
    ReplayModuleFrameBuffer metadata_buffer;
    auto frame_buffer = make_unique<uint16_t[]>(
            MODULE_N_PIXELS * REPLAY_READ_BUFFER_SIZE);

    ReplayH5Reader file_reader(device, channel_name, source_id, stop_pulse_id);

    uint64_t curr_pulse_id = start_pulse_id;
    // "<= stop_pulse_id" because we include the stop_pulse_id in the file.
    while (curr_pulse_id <= stop_pulse_id) {

        auto start_time = chrono::steady_clock::now();

        file_reader.get_buffer(curr_pulse_id,
                               &metadata_buffer,
                               (char *) (frame_buffer.get()));

        auto end_time = chrono::steady_clock::now();
        auto read_us_duration = chrono::duration_cast<chrono::microseconds>(
                end_time-start_time).count();

        start_time = chrono::steady_clock::now();

        zmq_send(socket,
                 &metadata_buffer,
                 sizeof(ReplayModuleFrameBuffer),
                 ZMQ_SNDMORE);
        zmq_send(socket,
                 (char*)(frame_buffer.get()),
                 metadata_buffer.data_n_bytes,
                 0);

        end_time = chrono::steady_clock::now();

        curr_pulse_id += metadata_buffer.n_frames;

        auto send_us_duration = chrono::duration_cast<chrono::microseconds>(
                end_time-start_time).count();
        auto avg_read_us = read_us_duration / metadata_buffer.n_frames;
        auto avg_send_us = send_us_duration / metadata_buffer.n_frames;

        cout << "sf_replay:avg_read_us " << avg_read_us;
        cout << " sf_replay:avg_send_us " << avg_send_us;
        cout << endl;
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

    sf_replay(
            socket, device, channel_name, source_id,
            start_pulse_id, stop_pulse_id);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
}
