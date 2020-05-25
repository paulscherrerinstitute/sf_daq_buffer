#include "ReplayH5Reader.hpp"

#include <iostream>
#include <thread>
#include <cstring>
#include <zmq.h>

#include "buffer_config.hpp"
#include "jungfrau.hpp"

using namespace std;
using namespace core_buffer;
using namespace chrono;

void sf_replay (
        void* socket,
        const string& device,
        const string& channel_name,
        const uint16_t source_id,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id)
{
    auto m_buffer = new ReplayModuleFrameBuffer();
    auto m_buffer_size = sizeof(ReplayModuleFrameBuffer);
    auto f_buffer = new char[MODULE_N_BYTES * REPLAY_READ_BUFFER_SIZE];

    ReplayH5Reader file_reader(device, channel_name, source_id, stop_pulse_id);

    uint64_t curr_pulse_id = start_pulse_id;
    // "<= stop_pulse_id" because we include the stop_pulse_id in the file.
    while (curr_pulse_id <= stop_pulse_id) {

        auto start_time = steady_clock::now();

        file_reader.get_buffer(curr_pulse_id, m_buffer, f_buffer);

        auto end_time = steady_clock::now();
        auto read_us_duration =
                duration_cast<microseconds>(end_time-start_time).count();

        start_time = steady_clock::now();

        zmq_send(socket, m_buffer, m_buffer_size, ZMQ_SNDMORE);
        zmq_send(socket, f_buffer, m_buffer->data_n_bytes, 0);

        end_time = steady_clock::now();

        curr_pulse_id += m_buffer->n_frames;

        auto send_us_duration =
                duration_cast<microseconds>(end_time-start_time).count();
        auto avg_read_us = read_us_duration / m_buffer->n_frames;
        auto avg_send_us = send_us_duration / m_buffer->n_frames;

        cout << "sf_replay:avg_read_us " << avg_read_us;
        cout << " sf_replay:avg_send_us " << avg_send_us;
        cout << endl;
    }

    delete[] f_buffer;
    delete m_buffer;
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
    const auto source_id = (uint16_t) atoi(argv[4]);
    const auto start_pulse_id = (uint64_t) atoll(argv[5]);
    const auto stop_pulse_id = (uint64_t) atoll(argv[6]);

    auto ipc_base = REPLAY_STREAM_IPC_URL + ipc_id + "-";
    stringstream ipc_stream;
    ipc_stream << ipc_base << (int)source_id;
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
