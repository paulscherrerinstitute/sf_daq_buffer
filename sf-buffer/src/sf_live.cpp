#include <iostream>
#include "jungfrau.hpp"
#include "zmq.h"
#include "buffer_config.hpp"
#include <cstring>
#include "date.h"
#include "LiveH5Reader.hpp"

using namespace std;
using namespace core_buffer;

void sf_live (
        void* socket,
        const string& device,
        const string& channel_name,
        const uint16_t source_id)
{
    LiveH5Reader reader(device, channel_name, source_id);

    auto current_pulse_id = reader.get_latest_pulse_id();
    while (true) {

        reader.load_pulse_id(current_pulse_id);

        auto metadata = reader.get_metadata();

        zmq_send(socket,
                 &metadata,
                 sizeof(ModuleFrame),
                 ZMQ_SNDMORE);

        auto data = reader.get_data();

        zmq_send(socket,
                 data,
                 MODULE_N_BYTES,
                 0);

        #ifdef DEBUG_OUTPUT
            using namespace date;
            using namespace chrono;

            cout << "[" << system_clock::now() << "]";
            cout << "[sf_live::sf_live]";
            cout << " Sent pulse_id ";
            cout << current_pulse_id << endl;
        #endif

        current_pulse_id++;
    }

    reader.close_file();
}

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_live [device] [channel_name] [source_id]";
        cout << endl;
        cout << "\tdevice: Name of detector." << endl;
        cout << "\tchannel_name: M00-M31 for JF16M." << endl;
        cout << "\tsource_id: Module index" << endl;
        cout << endl;

        exit(-1);
    }

    const string device = string(argv[1]);
    const string channel_name = string(argv[2]);
    const uint16_t source_id = (uint16_t) atoi(argv[3]);

    stringstream ipc_stream;
    ipc_stream << "ipc://sf-live-" << (int)source_id;
    const auto ipc_address = ipc_stream.str();

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[sf_live::main]";

        cout << " device " << device;
        cout << " channel_name " << channel_name;
        cout << " source_id " << source_id;
        cout << " start_pulse_id " << start_pulse_id;
        cout << " ipc_address " << ipc_address;
        cout << endl;
    #endif

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUSH);

    const int sndhwm = REPLAY_READ_BLOCK_SIZE;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(strerror (errno));

    const int linger_ms = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(strerror (errno));

    if (zmq_connect(socket, ipc_address.c_str()) != 0)
        throw runtime_error(strerror (errno));

    sf_live(socket, device, channel_name, source_id);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
}
