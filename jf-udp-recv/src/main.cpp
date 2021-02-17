#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "FrameUdpReceiver.hpp"
#include "BufferUtils.hpp"
#include "FrameStats.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace BufferUtils;



int main (int argc, char *argv[]) {

    if (argc != 3) {
        cout << endl;
        #ifndef USE_EIGER
            cout << "Usage: jf_udp_recv [detector_json_filename] [module_id]";
        #else
            cout << "Usage: eiger_udp_recv [detector_json_filename] [module_id]";
        #endif
        cout << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tmodule_id: id of the module for this process." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = read_json_config(string(argv[1]));
    const int module_id = atoi(argv[2]);

    const auto udp_port = config.start_udp_port + module_id;
    FrameUdpReceiver receiver(udp_port, module_id);
    RamBuffer buffer(config.detector_name, config.n_modules);
    FrameStats stats(config.detector_name, module_id, STATS_MODULO);

    auto ctx = zmq_ctx_new();
    auto socket = bind_socket(ctx, config.detector_name, module_id);

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
