#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "JfjFrameUdpReceiver.hpp"
#include "BufferUtils.hpp"
#include "JfjFrameStats.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace BufferUtils;

int main (int argc, char *argv[]) {

    if (argc != 3) {
        cout << endl;
        cout << "Usage: jfj_udp_recv [detector_json_filename]";
        cout << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = read_json_config(string(argv[1]));

    const auto udp_port = config.start_udp_port;
    JfjFrameUdpReceiver receiver(udp_port);
    RamBuffer buffer(config.detector_name, config.n_modules);
    FrameStats stats(config.detector_name, 0, STATS_TIME);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, ZMQ_IO_THREADS);
    auto sender = BufferUtils::bind_socket(ctx, config.detector_name, "jungfraujoch");

    // Might be better creating a structure for double buffering
    ModuleFrame frameMeta;
    ImageMetadata imageMeta;
    char* dataBuffer = new char[JFJOCH_DATA_BYTES_PER_FRAME];

    uint64_t pulse_id_previous = 0;
    uint64_t frame_index_previous = 0;


    while (true) {
        // NOTE: Needs to be pipelined for really high frame rates
        auto pulse_id = receiver.get_frame_from_udp(frameMeta, dataBuffer);

        bool bad_pulse_id = false;

        if ( ( frameMeta.frame_index != (frame_index_previous+1) ) || ( (pulse_id-pulse_id_previous) < 0 ) || ( (pulse_id-pulse_id_previous) > 1000 ) ) {
            bad_pulse_id = true;
        } else {
            imageMeta.pulse_id = frameMeta.pulse_id;
            imageMeta.frame_index = frameMeta.frame_index;
            imageMeta.daq_rec = frameMeta.daq_rec;
            imageMeta.is_good_image = true;            
            
            buffer.write_frame(frameMeta, dataBuffer);            
            zmq_send(sender, &imageMeta, sizeof(imageMeta), 0);
        }

        stats.record_stats(frameMeta, bad_pulse_id);

        pulse_id_previous = pulse_id;
        frame_index_previous = frameMeta.frame_index;

    }

    delete[] dataBuffer;
}
