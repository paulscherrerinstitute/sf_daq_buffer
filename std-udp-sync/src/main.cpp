#include <iostream>
#include <string>
#include <zmq.h>
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

int main (int argc, char *argv[])
{
    if (argc != 2) {
        cout << endl;
        cout << "Usage: std_udp_sync [detector_json_filename]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = UdpSyncConfig::from_json_file(string(argv[1]));

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, 1);

    auto sender = BufferUtils::bind_socket(ctx, config.detector_name, "sync");

    ZmqPulseSyncReceiver receiver(ctx, config.detector_name, config.n_modules);
    SyncStats stats(config.detector_name, STATS_TIME);

    while (true) {
        auto meta = receiver.get_next_pulse_id();

        zmq_send(sender, &meta.image_id, sizeof(meta.image_id), 0);

        stats.record_stats(meta.n_lost_pulses);
    }
}
