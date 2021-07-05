#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include <AssemblerStats.hpp>

#include <chrono>

#include "date.h"
#include "EigerAssembler.hpp"
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

    EigerAssembler assembler(n_receivers, bit_depth);

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [EigerAssembler] :";
        cout << " Eiger details:";
        cout << assembler;
        cout << endl;
    #endif

    RamBuffer frame_buffer(config.detector_name,
            sizeof(ModuleFrame), N_BYTES_PER_MODULE_FRAME(bit_depth), n_receivers,
            buffer_config::RAM_BUFFER_N_SLOTS);
    

    RamBuffer image_buffer(config.detector_name + "_" + stream_name,
            sizeof(ImageMetadata), assembler.get_image_n_bytes(), 1,
            buffer_config::RAM_BUFFER_N_SLOTS);

    ZmqPulseSyncReceiver receiver(ctx, config.detector_name, n_receivers);
    AssemblerStats stats(config.detector_name, ASSEMBLER_STATS_MODULO);
    
    while (true) {
        auto pulse_and_sync = receiver.get_next_pulse_id();
        // metadata
        auto* src_meta = frame_buffer.get_slot_meta(pulse_and_sync.pulse_id);
        auto* src_data = frame_buffer.get_slot_data(pulse_and_sync.pulse_id);
        // data
        auto* dst_meta = image_buffer.get_slot_meta(pulse_and_sync.pulse_id);
        auto* dst_data = image_buffer.get_slot_data(pulse_and_sync.pulse_id);
        // assemble 
        assembler.assemble_image(src_meta, src_data, dst_meta, dst_data);

        zmq_send(sender, dst_meta, sizeof(ImageMetadata), 0);

        stats.record_stats(
                (ImageMetadata*)dst_meta, pulse_and_sync.n_lost_pulses);
    }
}
