#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <chrono>
#include <sstream>
#include <zconf.h>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "jungfrau.hpp"
#include "BufferUdpReceiver.hpp"
#include "BufferBinaryWriter.hpp"

using namespace std;
using namespace chrono;
using namespace core_buffer;

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_buffer [detector_name] [device_name]";
        cout << " [udp_port] [root_folder] [source_id]";
        cout << endl;
        cout << "\tdetector_name: Detector name, example JF07T32V01" << endl;
        cout << "\tdevice_name: Name to write to disk.";
        cout << "\tudp_port: UDP port to connect to." << endl;
        cout << "\troot_folder: FS root folder." << endl;
        cout << "\tsource_id: ID of the source for live stream." << endl;
        cout << endl;

        exit(-1);
    }

    string detector_name = string(argv[1]);
    string device_name = string(argv[2]);
    int udp_port = atoi(argv[3]);
    string root_folder = string(argv[4]);
    int source_id = atoi(argv[5]);

    stringstream ipc_stream;
    string LIVE_IPC_URL = BUFFER_LIVE_IPC_URL + detector_name + "-";
    ipc_stream << LIVE_IPC_URL << source_id;
    const auto ipc_address = ipc_stream.str();

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUB);

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

    uint64_t stats_counter(0);
    uint64_t n_missed_packets = 0;
    uint64_t n_corrupted_frames = 0;

    BufferBinaryWriter writer(root_folder, device_name);
    BufferUdpReceiver receiver(udp_port, source_id);

    BufferBinaryFormat* binary_buffer = new BufferBinaryFormat();

    size_t write_total_us = 0;
    size_t write_max_us = 0;
    size_t send_total_us = 0;
    size_t send_max_us = 0;

    while (true) {

        auto pulse_id = receiver.get_frame_from_udp(
                binary_buffer->metadata, binary_buffer->data);

        auto start_time = steady_clock::now();

        writer.write(pulse_id, binary_buffer);

        auto write_end_time = steady_clock::now();
        auto write_us_duration = duration_cast<microseconds>(
                write_end_time-start_time).count();

        start_time = steady_clock::now();

        zmq_send(socket, &(binary_buffer->metadata), sizeof(ModuleFrame),
                ZMQ_SNDMORE);
        zmq_send(socket, binary_buffer->data, MODULE_N_BYTES, 0);

        auto send_end_time = steady_clock::now();
        auto send_us_duration = duration_cast<microseconds>(
                send_end_time-start_time).count();

        // TODO: Make real statistics, please.
        stats_counter++;
        write_total_us += write_us_duration;
        send_total_us += send_us_duration;

        if (write_us_duration > write_max_us) {
            write_max_us = write_us_duration;
        }

        if (send_us_duration > send_max_us) {
            send_max_us = send_us_duration;
        }

        if (binary_buffer->metadata.n_recv_packets < JF_N_PACKETS_PER_FRAME) {
            n_missed_packets += JF_N_PACKETS_PER_FRAME -
                    binary_buffer->metadata.n_recv_packets;
            n_corrupted_frames++;
        }

        if (stats_counter == STATS_MODULO) {
            cout << "sf_buffer:device_name " << device_name;
            cout << " sf_buffer:n_missed_packets " << n_missed_packets;
            cout << " sf_buffer:n_corrupted_frames " << n_corrupted_frames;

            cout << " sf_buffer:write_total_us " << write_total_us/STATS_MODULO;
            cout << " sf_buffer:write_max_us " << write_max_us;
            cout << " sf_buffer:send_total_us " << send_total_us/STATS_MODULO;
            cout << " sf_buffer:send_max_us " << send_max_us;
            cout << endl;

            stats_counter = 0;
            n_missed_packets = 0;
            n_corrupted_frames = 0;

            write_total_us = 0;
            write_max_us = 0;
            send_total_us = 0;
            send_max_us = 0;
        }
    }

    delete binary_buffer;
}
