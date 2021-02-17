#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include "live_writer_config.hpp"
#include "../../jf-buffer-writer/include/BufferStats.hpp"


using namespace std;
using namespace buffer_config;
using namespace live_writer_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        cout << "Usage: jf_live_writer [detector_json_filename]"
                " [stream_name]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << endl;

        exit(-1);
    }

    const auto stream_name = string(argv[2]);
    auto config = BufferUtils::read_json_config(string(argv[1]));

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, LIVE_ZMQ_IO_THREADS);
    auto receiver = BufferUtils::connect_socket(
            ctx, config.detector_name, "assembler");

    RamBuffer ram_buffer(config.detector_name, config.n_modules);
    BufferStats stats(config.detector_name, stream_name, STATS_MODULO);

    ImageMetadata meta;
    while (true) {
        zmq_recv(receiver, &meta, sizeof(meta), 0);
        char* data = ram_buffer.read_image(meta.pulse_id);

        sender.send(meta, data);

        stats.record_stats(meta);
    }
}
