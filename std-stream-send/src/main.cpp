#include <iostream>
#include <zmq.h>
#include "stream_config.hpp"
#include <chrono>
#include <thread>
#include "RamBuffer.hpp"


using namespace std;
using namespace stream_config;

int main (int argc, char *argv[])
{
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

    if (zmq_bind(sender, "tcp://127.0.0.1:10000") != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    RamBuffer image_buffer(config.detector_name + "_assembler",
            sizeof(ImageMetadata), assembler.get_image_n_bytes(), 1);

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
