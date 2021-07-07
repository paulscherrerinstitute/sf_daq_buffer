#include <iostream>
#include <zmq.h>
#include "stream_config.hpp"
#include <chrono>
#include <thread>
#include <StreamSendConfig.hpp>
#include "RamBuffer.hpp"


using namespace std;
using namespace stream_config;
using namespace buffer_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        cout << "Usage: std_stream_send [detector_json_filename]"
                " [bit_depth] [stream_address]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tbit_depth: bit depth of the incoming udp packets." << endl;
        cout << "\tstream_address: address to bind the output stream." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = StreamSendConfig::from_json_file(string(argv[1]));
    const int bit_depth = atoi(argv[2]);
    const string stream_address = string(argv[3]);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);

    auto sender = zmq_socket(ctx, ZMQ_PUSH);
    const int sndhwm = PROCESSING_ZMQ_SNDHWM;
    if (zmq_setsockopt(
            sender, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    const int linger = 0;
    if (zmq_setsockopt(
            sender, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_bind(sender, stream_address.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    const size_t IMAGE_N_BYTES = config.image_n_pixels * bit_depth / 8;
    RamBuffer image_buffer(config.detector_name + "_assembler",
            sizeof(ImageMetadata), IMAGE_N_BYTES,
            config.n_modules, RAM_BUFFER_N_SLOTS);

    while (true) {

        image_id = 123;

        zmq_send(sender,
                 ram_buffer.get_slot_meta(image_id),
                 sizeof(ImageMetadata), ZMQ_SNDMORE);

        zmq_send(sender,
                 ram_buffer.get_slot_data(image_id),
                 buffer_config::MODULE_N_BYTES * 4, 0);

        pulse_id++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
