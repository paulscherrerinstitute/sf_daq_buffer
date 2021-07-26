#include <iostream>
#include <string>
#include <zmq.h>
#include "date.h"
#include <chrono>

#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include <AssemblerStats.hpp>

#include "EigerAssembler.hpp"
#include "assembler_config.hpp"
#include "ZmqPulseSyncReceiver.hpp"

#ifdef USE_EIGER
#include "eiger.hpp"
#else
#include "jungfrau.hpp"
#endif

using namespace std;
using namespace buffer_config;
using namespace assembler_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        #ifndef USE_EIGER
            cout << "Usage: jf_assembler [detector_json_filename] "
            " [bit_depth]" << endl;
        #else
            cout << "Usage: eiger_assembler [detector_json_filename] "
            " [bit_depth]" << endl;
        #endif
        cout << "\tdetector_json_filename: detector config file path.";
        cout << endl;
        cout << "\tbit_depth: bit depth of the image.";
        cout << endl;

        exit(-1);
    }

    auto config = BufferUtils::read_json_config(string(argv[1]));
    const int bit_depth = atoi(argv[2]);
    auto const stream_name = "assembler";

    const size_t IMAGE_N_BYTES = config.image_height * config.image_width * bit_depth / 8;
    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, ASSEMBLER_ZMQ_IO_THREADS);
    auto sender = BufferUtils::bind_socket(
            ctx, config.detector_name, stream_name);
    auto receiver_sync = BufferUtils::connect_socket(
            ctx, config.detector_name, "sync");

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [Assembler] :";
        cout << " Details of Assembler:";
        cout << " detector_name: " << config.detector_name;
        cout << " || n_modules: " << config.n_modules;
        cout << " || img width: " << config.image_width;
        cout << " || img height: " << config.image_height;
        cout << endl;
    #endif

    const size_t FRAME_N_BYTES = MODULE_N_PIXELS * bit_depth / 8;
    const size_t N_PACKETS_PER_FRAME = FRAME_N_BYTES / DATA_BYTES_PER_PACKET;

    EigerAssembler assembler(config.n_modules, bit_depth, 
            config.image_width, config.image_height);

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [EigerAssembler] :";
        cout << " Eiger details:";
        cout << assembler;
        cout << endl;
    #endif

    RamBuffer frame_buffer(config.detector_name,
            sizeof(ModuleFrame), FRAME_N_BYTES, config.n_modules,
            buffer_config::RAM_BUFFER_N_SLOTS);
    

    RamBuffer image_buffer(config.detector_name + "_" + stream_name,
            sizeof(ImageMetadata), IMAGE_N_BYTES, 1,
            buffer_config::RAM_BUFFER_N_SLOTS);

    AssemblerStats stats(config.detector_name, STATS_TIME);


    uint64_t image_id = 0;

    while (true) {
        // receives the synced image id
        zmq_recv(receiver_sync, &image_id, sizeof(image_id), 0);

        // metadata
        auto* src_meta = frame_buffer.get_slot_meta(image_id);
        auto* src_data = frame_buffer.get_slot_data(image_id);
        // data
        auto* dst_meta = image_buffer.get_slot_meta(image_id);
        auto* dst_data = image_buffer.get_slot_data(image_id);
        // assemble 
        assembler.assemble_image(src_meta, src_data, dst_meta, dst_data);

        zmq_send(sender, dst_meta, sizeof(ImageMetadata), 0);

        stats.record_stats(
                (ImageMetadata*)dst_meta, assembler.get_last_img_status());
    }
}
