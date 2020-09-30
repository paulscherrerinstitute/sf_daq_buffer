#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferStats.hpp>

#include "formats.hpp"
#include "BufferUtils.hpp"
#include "buffer_config.hpp"
#include "jungfrau.hpp"
#include "BufferBinaryWriter.hpp"

using namespace std;
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
    RamBuffer ram_buff(detector_name, n_modules);
    auto file_buff = new BufferBinaryFormat();

    BufferStats stats(device_name, STATS_MODULO);

    auto ctx = zmq_ctx_new();
    auto socket = BufferUtils::connect_socket(ctx, detector_name, source_id);

    uint64_t pulse_id;
    while (true) {

        zmq_recv(socket, &pulse_id, sizeof(pulse_id), 0);

        stats.start_frame_write();

        // TODO: Memory copy here. Optimize this one out.
        ram_buff.read_frame(
                pulse_id, source_id, file_buff->meta, file_buff->data);

        writer.write(pulse_id, file_buff);

        stats.end_frame_write();
    }
}
