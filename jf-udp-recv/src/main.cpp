#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "FrameUdpReceiver.hpp"
#include "BufferUtils.hpp"
#include "FrameStats.hpp"
 
using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace BufferUtils;



int main (int argc, char *argv[]) {

    if (argc != 4) {
        cout << endl;
        #ifndef USE_EIGER
            cout << "Usage: jf_udp_recv [detector_json_filename] [module_id] [bit_depth]";
        #else
            cout << "Usage: eiger_udp_recv [detector_json_filename] [module_id] [bit_depth]";
        #endif
        cout << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tmodule_id: id of the module for this process." << endl;
        cout << "\tbit_depth: bit depth of the incoming udp packets." << endl;
        cout << endl;
        exit(-1);
    }

    const auto config = read_json_config(string(argv[1]));
    const int module_id = atoi(argv[2]);
    const int bit_depth = atoi(argv[3]);
    const int n_receivers = config.n_modules * config.n_submodules;
    const auto udp_port = config.start_udp_port + module_id;
    
    FrameUdpReceiver receiver(module_id, udp_port, n_receivers, config.n_submodules, bit_depth);
    RamBuffer buffer(config.detector_name, n_receivers, config.n_submodules, bit_depth);
    FrameStats stats(config.detector_name, n_receivers, module_id, bit_depth, STATS_TIME);

    auto ctx = zmq_ctx_new();
    auto socket = bind_socket(ctx, config.detector_name, to_string(module_id));

    ModuleFrame meta;
    // TODO: This will not work. Only if Eiger sends in 16 bit. Use MODULE_N_PIXELS * bit_depth / 8
    char* data = new char[MODULE_N_BYTES];

    uint64_t pulse_id_previous = 0;
    uint64_t frame_index_previous = 0; 
    
    while (true) {

        auto pulse_id = receiver.get_frame_from_udp(meta, data);

        bool bad_pulse_id = false;       
        if ( ( meta.frame_index != (frame_index_previous+1) ) ||
             ( (meta.frame_index-frame_index_previous) <= 0 ) ||
             ( (meta.frame_index-frame_index_previous) > 1000 ) ){  
            bad_pulse_id = true;
        } else { 
            buffer.write_frame(meta, data);

            zmq_send(socket, &pulse_id, sizeof(pulse_id), 0);

        }

        stats.record_stats(meta, bad_pulse_id);

        pulse_id_previous = pulse_id;
        frame_index_previous = meta.frame_index;

    }

    delete[] data;
}
