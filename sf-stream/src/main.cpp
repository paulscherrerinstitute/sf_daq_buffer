#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include <StreamStats.hpp>

#include "stream_config.hpp"
#include "ZmqLiveSender.hpp"

using namespace std;
using namespace buffer_config;
using namespace stream_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        cout << "Usage: sf_stream [detector_json_filename]"
                " [stream_name]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tstream_name: name of the stream." << endl;
        cout << endl;

        exit(-1);
    }

    const auto stream_name = string(argv[2]);
    // TODO: Add stream_name to config reading - multiple stream definitions.
    auto config = BufferUtils::read_json_config(string(argv[1]));

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);
    auto receiver = BufferUtils::connect_socket(
            ctx, config.detector_name, "assembler");

    RamBuffer ram_buffer(config.detector_name, config.n_modules);
    StreamStats stats(config.detector_name, stream_name, STREAM_STATS_MODULO);
    ZmqLiveSender sender(ctx, config);

    ImageMetadata meta;
    while (true) {
        zmq_recv(receiver, &meta, sizeof(meta), 0);
        char* data = ram_buffer.read_image(meta.frame_index);
        

        sender.send(meta, data);

        stats.record_stats(meta);
    }
}
