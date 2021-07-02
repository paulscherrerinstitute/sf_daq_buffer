#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include <AssemblerStats.hpp>

#include "date.h"
#include <chrono>
#include "assembler_config.hpp"
#include "ZmqPulseSyncReceiver.hpp"
 

using namespace std;
using namespace buffer_config;
using namespace assembler_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        #ifndef USE_EIGER
            cout << "Usage: jf_assembler [detector_json_filename] [bit_depth]" << endl;
        #else
            cout << "Usage: eiger_assembler [detector_json_filename] [bit_depth]" << endl;
        #endif
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tbit_depth: bit depth of the incoming udp packets." << endl;
        cout << endl;

        exit(-1);
    }

    auto config = BufferUtils::read_json_config(string(argv[1]));
    const int bit_depth = atoi(argv[2]);
    auto const stream_name = "assembler";

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, ASSEMBLER_ZMQ_IO_THREADS);
    auto sender = BufferUtils::bind_socket(
            ctx, config.detector_name, stream_name);
    const int n_receivers =  config.n_modules * config.n_submodules;

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [Assembler] :";
        cout << " Details of Assembler:";
        cout << " detector_name: " << config.detector_name;
        cout << " || n_modules: " << config.n_modules;
        cout << " || n_receivers: " << n_receivers;
        cout << endl;
    #endif

    

    ZmqPulseSyncReceiver receiver(ctx, config.detector_name, n_receivers);
    RamBuffer ram_buffer(config.detector_name, n_receivers, config.n_submodules, bit_depth);
    AssemblerStats stats(config.detector_name, ASSEMBLER_STATS_MODULO);
    
    ImageMetadata meta;

    while (true) {
        auto pulse_and_sync = receiver.get_next_pulse_id();
        ram_buffer.assemble_image(pulse_and_sync.pulse_id, meta);

        zmq_send(sender, &meta, sizeof(meta), 0);

        stats.record_stats(meta, pulse_and_sync.n_lost_pulses);
    }
}
