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
#include "BufferUtils.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_buffer_recv [detector_name] [n_modules]";
        cout << " [device_name] [udp_port] [root_folder] [source_id]";
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
    auto stats_interval_start = steady_clock::now();

    FrameUdpReceiver receiver(udp_port, source_id);
    RamBuffer buffer(detector_name, n_modules);

    auto ctx = zmq_ctx_new();
    auto socket = BufferUtils::bind_socket(ctx, detector_name, source_id);

    while (true) {

        auto pulse_id = receiver.get_frame_from_udp(
                binary_buffer->metadata, binary_buffer->data);

        buffer.write_frame(&(binary_buffer->metadata),
                           &(binary_buffer->data[0]));

        zmq_send(socket, &pulse_id, sizeof(pulse_id), 0);

        // TODO: Isolate in a class.
        if (binary_buffer->metadata.n_recv_packets < JF_N_PACKETS_PER_FRAME) {
            n_missed_packets += JF_N_PACKETS_PER_FRAME -
                    binary_buffer->metadata.n_recv_packets;
            n_corrupted_frames++;
        }

        stats_counter++;
        if (stats_counter == STATS_MODULO) {
            auto interval_ms_duration = duration_cast<milliseconds>(
                    stats_interval_start-steady_clock::now()).count();
            // * 1000 because milliseconds, 0.5 for truncation.
            int rep_rate = ((stats_counter/interval_ms_duration) * 1000) + 0.5;

            cout << "sf_buffer:device_name " << device_name;
            cout << " sf_buffer:n_missed_packets " << n_missed_packets;
            cout << " sf_buffer:n_corrupted_frames " << n_corrupted_frames;
            cout << " sf_buffer:repetition_rate " << rep_rate;
            cout << endl;

            stats_counter = 0;
            n_missed_packets = 0;
            n_corrupted_frames = 0;
            stats_interval_start = steady_clock::now();
        }
    }
}