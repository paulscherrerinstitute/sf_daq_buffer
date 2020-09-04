#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <chrono>
#include <sstream>
#include <zconf.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "jungfrau.hpp"
#include "FrameUdpReceiver.hpp"
#include "BufferBinaryWriter.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;

void* get_live_stream_socket(const string& detector_name, const int source_id)
{
    stringstream ipc_stream;
    string LIVE_IPC_URL = BUFFER_LIVE_IPC_URL + detector_name + "-";
    ipc_stream << LIVE_IPC_URL << source_id;
    const auto ipc_address = ipc_stream.str();

    void* ctx = zmq_ctx_new();
    void* socket = zmq_socket(ctx, ZMQ_PUB);

    const int sndhwm = BUFFER_ZMQ_SNDHWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    const int linger = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_bind(socket, ipc_address.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    return socket;
}

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_buffer [detector_name] [n_modules] [device_name]";
        cout << " [udp_port] [root_folder] [source_id]";
        cout << endl;
        cout << "\tdetector_name: Detector name, example JF07T32V01" << endl;
        cout << "\tn_modules: Number of modules in the detector." << endl;
        cout << "\tdevice_name: Name to write to disk." << endl;
        cout << "\tudp_port: UDP port to connect to." << endl;
        cout << "\troot_folder: FS root folder." << endl;
        cout << "\tsource_id: ID of the source for live stream." << endl;
        cout << endl;

        exit(-1);
    }

    string detector_name = string(argv[1]);
    int n_modules = atoi(argv[2]);
    string device_name = string(argv[3]);
    int udp_port = atoi(argv[4]);
    string root_folder = string(argv[5]);
    int source_id = atoi(argv[6]);

    uint64_t stats_counter(0);
    uint64_t n_missed_packets = 0;
    uint64_t n_corrupted_frames = 0;

    BufferBinaryWriter writer(root_folder, device_name);
    FrameUdpReceiver receiver(udp_port, source_id);
    RamBuffer buffer(detector_name, n_modules);

    auto binary_buffer = new BufferBinaryFormat();
    auto socket = get_live_stream_socket(detector_name, source_id);

    while (true) {

        auto pulse_id = receiver.get_frame_from_udp(
                binary_buffer->metadata, binary_buffer->data);

        writer.write(pulse_id, binary_buffer);

        buffer.write_frame(&(binary_buffer->metadata),
                           &(binary_buffer->data[0]));

        zmq_send(socket, &pulse_id, sizeof(pulse_id), 0);

        if (binary_buffer->metadata.n_recv_packets < JF_N_PACKETS_PER_FRAME) {
            n_missed_packets += JF_N_PACKETS_PER_FRAME -
                    binary_buffer->metadata.n_recv_packets;
            n_corrupted_frames++;
        }

        stats_counter++;
        if (stats_counter == STATS_MODULO) {
            cout << "sf_buffer:device_name " << device_name;
            cout << " sf_buffer:n_missed_packets " << n_missed_packets;
            cout << " sf_buffer:n_corrupted_frames " << n_corrupted_frames;
            cout << endl;

            stats_counter = 0;
            n_missed_packets = 0;
            n_corrupted_frames = 0;
        }
    }
}
