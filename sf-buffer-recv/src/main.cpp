#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <zconf.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "FrameUdpReceiver.hpp"
#include "BufferUtils.hpp"
#include "FrameStats.hpp"

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

    FrameUdpReceiver receiver(udp_port, source_id);
    RamBuffer buffer(detector_name, n_modules);

    auto ctx = zmq_ctx_new();
    auto socket = BufferUtils::bind_socket(ctx, detector_name, source_id);

    FrameStats stats(device_name, STATS_MODULO);

    ModuleFrame meta;
    char* data = new char[MODULE_N_BYTES];

    while (true) {
        auto pulse_id = receiver.get_frame_from_udp(meta, data);

        buffer.write_frame(meta, data);

        zmq_send(socket, &pulse_id, sizeof(pulse_id), 0);

        stats.record_stats(meta);
    }

    delete[] data;
}
