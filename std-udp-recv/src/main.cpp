#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "FrameUdpReceiver.hpp"
#include "BufferUtils.hpp"
#include "FrameStats.hpp"
#include "UdpRecvConfig.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace BufferUtils;


int main (int argc, char *argv[]) {

    if (argc != 4) {
        cout << endl;
        cout << "Usage: std_udp_recv [detector_json_filename] [module_id] "
                "[bit_depth]";
        cout << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tmodule_id: id of the module for this process." << endl;
        cout << "\tbit_depth: bit depth of the incoming udp packets." << endl;
        cout << endl;
        exit(-1);
    }

    const auto config = UdpRecvConfig::from_json_file(string(argv[1]));
    const int module_id = atoi(argv[2]);
    const int bit_depth = atoi(argv[3]);

    if (DETECTOR_TYPE != config.detector_type) {
        throw runtime_error("UDP recv version for " + DETECTOR_TYPE +
                            " but config for " + config.detector_type);
    }

    const auto udp_port = config.start_udp_port + module_id;
    const size_t FRAME_N_BYTES = MODULE_N_PIXELS * bit_depth / 8;
    const size_t N_PACKETS_PER_FRAME = FRAME_N_BYTES / DATA_BYTES_PER_PACKET;

    FrameUdpReceiver receiver(udp_port, N_PACKETS_PER_FRAME);
    RamBuffer frame_buffer(config.detector_name, sizeof(ModuleFrame),
                           FRAME_N_BYTES, config.n_modules, RAM_BUFFER_N_SLOTS);
    FrameStats stats(config.detector_name, module_id,
            N_PACKETS_PER_FRAME, STATS_TIME);

    auto ctx = zmq_ctx_new();
    auto socket = bind_socket(ctx, config.detector_name, to_string(module_id));

    ModuleFrame meta;
    meta.module_id = module_id;
    meta.bit_depth = bit_depth;

    char* data = new char[FRAME_N_BYTES];

    while (true) {
        // Reset the metadata and frame buffer for the next frame.
        meta.frame_index = 0;
        meta.n_recv_packets = 0;
        // Reset the data buffer.
        memset(data, 0, FRAME_N_BYTES);

        receiver.get_frame_from_udp(meta, data);

        // Assign the image_id based on the detector type.
#ifdef USE_EIGER
        const uint64_t image_id = meta.frame_index;
#else
        const uint64_t image_id = meta.pulse_id;
#endif
        meta.id = image_id;

        frame_buffer.write_frame(meta, data);
        zmq_send(socket, &image_id, sizeof(image_id), 0);

        stats.record_stats(meta);
    }

    delete[] data;
}
