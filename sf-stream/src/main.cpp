#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>

#include "buffer_config.hpp"
#include "stream_config.hpp"
#include "ZmqLiveSender.hpp"
#include "ZmqPulseSyncReceiver.hpp"

using namespace std;
using namespace buffer_config;
using namespace stream_config;

int main (int argc, char *argv[])
{
    if (argc != 1) {
        cout << endl;
        cout << "Usage: sf_stream [detector_json_filename]" << endl;
        cout << "\tdetector_json_filename: json file with the configuration "
                "of the detector." << endl;
        cout << endl;

        exit(-1);
    }

    auto config = BufferUtils::read_json_config(string(argv[1]));
    string RECV_IPC_URL = BUFFER_LIVE_IPC_URL + config.DETECTOR_NAME + "-";

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);

    ZmqPulseSyncReceiver receiver(ctx, config.DETECTOR_NAME, config.n_modules);
    RamBuffer ram_buffer(config.DETECTOR_NAME, config.n_modules);
    ZmqLiveSender sender(ctx, config);

    // TODO: Remove stats trash.
    uint64_t last_pulse_id = 0;
    uint64_t last_pulse_id_range = 0;
    uint16_t n_good_images = 0;

    ImageMetadata meta;
    while (true) {
        auto pulse_id = receiver.get_next_pulse_id();
        char* data = ram_buffer.read_image(pulse_id, meta);

        sender.send(meta, data);

        // TODO: This logic works only at 100Hz. Fix it systematically.
        uint64_t sync_lost_pulses = pulse_id - last_pulse_id;
        if (last_pulse_id > 0 && sync_lost_pulses > 1) {
            cout << "sf_stream:sync_lost_pulses " << sync_lost_pulses << endl;
        }
        last_pulse_id = pulse_id;

        uint64_t curr_pulse_id_range = pulse_id / 10000;
        if (last_pulse_id_range != curr_pulse_id_range) {
            if (last_pulse_id_range > 0) {
                cout << "sf_stream:n_good_images " << n_good_images;
                cout << endl;
            }

            last_pulse_id_range = curr_pulse_id_range;
            n_good_images = 0;
        }

        if (meta.is_good_image) {
            n_good_images++;
        }
    }
}
