#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include <ImageBinaryWriter.hpp>
#include "live_writer_config.hpp"
#include "WriterStats.hpp"


using namespace std;
using namespace buffer_config;
using namespace live_writer_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        cout << "Usage: jf_live_writer [detector_json_filename]"
                " [writer_id]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\twriter_id: Index of this writer instance." << endl;
        cout << endl;

        exit(-1);
    }

    auto config = BufferUtils::read_json_config(string(argv[1]));
    const int writer_id = atoi(argv[2]);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, LIVE_ZMQ_IO_THREADS);
    auto receiver = BufferUtils::connect_socket(
            ctx, config.detector_name, "assembler");

    RamBuffer ram_buffer(config.detector_name, config.n_modules);

    const uint64_t image_n_bytes = config.n_modules * MODULE_N_BYTES;
    ImageBinaryWriter writer(config.detector_name, image_n_bytes);

    WriterStats stats(config.detector_name, STATS_MODULO, image_n_bytes);

    ImageMetadata meta = {};
    while (true) {
        zmq_recv(receiver, &meta, sizeof(meta), 0);
        char* data = ram_buffer.read_image(meta.pulse_id);

        stats.start_image_write();

        writer.write(meta, data);

        stats.end_image_write();
    }
}
