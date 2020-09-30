#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferStats.hpp>

#include "formats.hpp"
#include "BufferUtils.hpp"
#include "buffer_config.hpp"
#include "jungfrau.hpp"
#include "BufferBinaryWriter.hpp"

using namespace std;
using namespace buffer_config;
using namespace BufferUtils;

int main (int argc, char *argv[]) {

    if (argc != 2) {
        cout << endl;
        cout << "Usage: jf_buffer_writer [detector_json_filename] [module_id]";
        cout << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tmodule_id: id of the module for this process." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = read_json_config(string(argv[1]));
    const int module_id = atoi(argv[2]);

    const auto module_name = "M" + to_string(module_id);
    BufferBinaryWriter writer(config.buffer_folder, module_name);
    RamBuffer ram_buff(config.detector_name, config.n_modules);
    BufferStats stats(module_name, STATS_MODULO);

    auto ctx = zmq_ctx_new();
    auto socket = connect_socket(ctx, config.detector_name, module_id);

    auto file_buff = new BufferBinaryFormat();
    uint64_t pulse_id;

    while (true) {

        zmq_recv(socket, &pulse_id, sizeof(pulse_id), 0);

        stats.start_frame_write();

        // TODO: Memory copy here. Optimize this one out.
        ram_buff.read_frame(
                pulse_id, module_id, file_buff->meta, file_buff->data);

        writer.write(pulse_id, file_buff);

        stats.end_frame_write();
    }
}
