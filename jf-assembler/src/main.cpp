#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include <AssemblerStats.hpp>

#include "assembler_config.hpp"
#include "ZmqPulseSyncReceiver.hpp"

using namespace std;
using namespace buffer_config;
using namespace assembler_config;

int main (int argc, char *argv[])
{
    if (argc != 2) {
        cout << endl;
        cout << "Usage: jf_assembler [detector_json_filename]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << endl;

        exit(-1);
    }

    auto config = BufferUtils::read_json_config(string(argv[1]));
    auto const stream_name = "assembler";

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, ASSEMBLER_ZMQ_IO_THREADS);
    auto sender = BufferUtils::bind_socket(
            ctx, config.detector_name, stream_name);

    ZmqPulseSyncReceiver receiver(ctx, config.detector_name, config.n_modules);
    RamBuffer ram_buffer(config.detector_name, config.n_modules);
    AssemblerStats stats(config.detector_name, ASSEMBLER_STATS_MODULO);

    ImageMetadata meta;
    while (true) {
        auto pulse_and_sync = receiver.get_next_pulse_id();
        ram_buffer.assemble_image(pulse_and_sync.pulse_id, meta);

        zmq_send(sender, &meta, sizeof(meta), 0);

        stats.record_stats(meta, pulse_and_sync.n_lost_pulses);
    }
}
