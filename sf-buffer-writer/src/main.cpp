#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <chrono>
#include <sstream>
#include <zconf.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "BufferUtils.hpp"
#include "buffer_config.hpp"
#include "jungfrau.hpp"
#include "BufferBinaryWriter.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_buffer_writer [detector_name] [n_modules]";
        cout << " [device_name] [root_folder] [source_id]";
        cout << endl;
        cout << "\tdetector_name: Detector name, example JF07T32V01" << endl;
        cout << "\tn_modules: Number of modules in the detector." << endl;
        cout << "\tdevice_name: Name to write to disk." << endl;
        cout << "\troot_folder: FS root folder." << endl;
        cout << "\tsource_id: ID of the source for live stream." << endl;
        cout << endl;

        exit(-1);
    }

    string detector_name = string(argv[1]);
    int n_modules = atoi(argv[2]);
    string device_name = string(argv[3]);
    string root_folder = string(argv[4]);
    int source_id = atoi(argv[5]);

    BufferBinaryWriter writer(root_folder, device_name);
    RamBuffer buffer(detector_name, n_modules);

    auto binary_buffer = new BufferBinaryFormat();

    auto ctx = zmq_ctx_new();
    auto socket = BufferUtils::connect_socket(ctx, detector_name, source_id);

    ImageMetadata* meta = nullptr;
    char* data = nullptr;

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
