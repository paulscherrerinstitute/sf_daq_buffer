#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include <SyncStats.hpp>

#include "date.h"
#include <chrono>
#include "sync_config.hpp"
#include "ZmqPulseSyncReceiver.hpp"
#include "UdpSyncConfig.hpp"
#include "buffer_config.hpp"
 

using namespace std;
using namespace sync_config;
using namespace buffer_config;

#ifdef USE_EIGER
    #include "eiger.hpp"
#else
    #include "jungfrau.hpp"
#endif

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        cout << "Usage: std_udp_sync [detector_json_filename] [bit_depth]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tbit_depth: bit depth of the incoming udp packets." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = UdpSyncConfig::from_json_file(string(argv[1]));
    const int bit_depth = atoi(argv[2]);

    const size_t FRAME_N_BYTES = MODULE_N_PIXELS * bit_depth / 8;

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, 1);

    auto sender = BufferUtils::bind_socket(ctx, config.detector_name, "sync");

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [Assembler] :";
        cout << " Details of Assembler:";
        cout << " detector_name: " << config.detector_name;
        cout << " || n_modules: " << config.n_modules;
        cout << endl;
    #endif

    RamBuffer frame_buffer(config.detector_name, sizeof(ModuleFrame),
                           FRAME_N_BYTES, config.n_modules, RAM_BUFFER_N_SLOTS);

    ZmqPulseSyncReceiver receiver(ctx, config.detector_name, config.n_modules);
    SyncStats stats(config.detector_name, SYNC_STATS_MODULO);

    while (true) {
        auto meta = receiver.get_next_pulse_id();

        zmq_send(sender, &meta.image_id, sizeof(meta.image_id), 0);

        stats.record_stats(meta.n_lost_pulses);
    }
}
