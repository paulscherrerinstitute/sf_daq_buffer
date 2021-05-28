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
        cout << "Usage: jf_udp_recv [detector_json_filename] [module_id]";
        cout << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tmodule_id: id of the module for this process." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = read_json_config(string(argv[1]));
    const int module_id = atoi(argv[2]);

    const auto udp_port = config.start_udp_port + module_id;
    ImageUdpReceiver receiver(udp_port, module_id);
    RamBuffer buffer(config.detector_name, config.n_modules);
    ImageStats stats(config.detector_name, module_id, STATS_TIME);

    auto ctx = zmq_ctx_new();
    auto socket = bind_socket(ctx, config.detector_name, to_string(module_id));


    // Might be better creating a structure for double buffering
    ImageMetadata metaBufferA;
    char* dataBufferA = new char[IMAGE_N_BYTES];

    uint64_t pulse_id_previous = 0;
    uint64_t frame_index_previous = 0;


    while (true) {
        // NOTE: Needs to be pipelined for really high frame rates
        auto pulse_id = receiver.get_image_from_udp(metaBufferA, dataBufferA);

        bool bad_pulse_id = false;

        if ( ( metaBufferA.frame_index != (frame_index_previous+1) ) ||
             ( (pulse_id-pulse_id_previous) < 0 ) ||
             ( (pulse_id-pulse_id_previous) > 1000 ) ) {

            bad_pulse_id = true;

        } else {

            buffer.write_frame(metaBufferA, dataBufferA);

            zmq_send(socket, &pulse_id, sizeof(pulse_id), 0);

        }

        stats.record_stats(metaBufferA, bad_pulse_id);

        pulse_id_previous = pulse_id;
        frame_index_previous = metaBufferA.frame_index;

    }

    delete[] data;
}
