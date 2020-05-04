#include <iostream>
#include <stdexcept>
#include <RingBuffer.hpp>
#include <UdpRecvModule.hpp>
#include <FastH5Writer.hpp>
#include "zmq.h"
#include "buffer_config.hpp"
#include "jungfrau.hpp"


using namespace std;
using namespace core_buffer;


int main (int argc, char *argv[]) {
    if (argc != 4) {
        cout << endl;
        cout << "Usage: sf_buffer [device_name] [udp_port] [root_folder]";
        cout << "[source_id]";
        cout << endl;
        cout << "\tdevice_name: Name to write to disk.";
        cout << "\tudp_port: UDP port to connect to." << endl;
        cout << "\troot_folder: FS root folder." << endl;
        cout << "\tsource_id: ID of the source for live stream." << endl;
        cout << endl;

        exit(-1);
    }

    string device_name = string(argv[1]);
    int udp_port = atoi(argv[2]);
    string root_folder = string(argv[3]);
    int source_id = atoi(argv[2]);

    stringstream ipc_stream;
    ipc_stream << "ipc://sf-live-" << source_id;
    const auto ipc_address = ipc_stream.str();

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUB);

    const int sndhwm = BUFFER_LIVE_SEND_HWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(strerror (errno));

    const int linger_ms = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(strerror (errno));

    if (zmq_connect(socket, ipc_address.c_str()) != 0)
        throw runtime_error(strerror (errno));

    RingBuffer<UdpFrameMetadata> ring_buffer(BUFFER_RB_SIZE);

    UdpRecvModule udp_module(ring_buffer);
    udp_module.start_recv(udp_port, JUNGFRAU_DATA_BYTES_PER_FRAME);

    uint64_t stats_counter(0);
    uint64_t n_missed_packets = 0;
    uint64_t n_missed_frames = 0;
    uint64_t last_pulse_id = 0;

    FastH5Writer writer(
            core_buffer::FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE,
            device_name, root_folder);

    writer.add_scalar_metadata<uint64_t>("pulse_id");
    writer.add_scalar_metadata<uint64_t>("frame_id");
    writer.add_scalar_metadata<uint32_t>("daq_rec");
    writer.add_scalar_metadata<uint16_t>("received_packets");

    while (true) {
        auto data = ring_buffer.read();

        if (data.first == nullptr) {
            this_thread::sleep_for(chrono::milliseconds(10));
            continue;
        }

        auto pulse_id = data.first->pulse_id;
        writer.set_pulse_id(pulse_id);

        writer.write_data(data.second);

        // TODO: Combine all this into 1 struct.

        writer.write_scalar_metadata<uint64_t>(
                "pulse_id", &(data.first->pulse_id));

        writer.write_scalar_metadata<uint64_t>(
                "frame_id",
                &(data.first->frame_index));

        writer.write_scalar_metadata<uint32_t>(
                "daq_rec",
                &(data.first->daq_rec));

        writer.write_scalar_metadata<uint16_t>(
                "received_packets",
                &(data.first->n_recv_packets));

        ModuleFrame metadata = {
                metadata.pulse_id,
                metadata.frame_index,
                metadata.daq_rec,
                metadata.n_received_packets,
                (uint16_t) source_id
        };

        zmq_send(socket,
                 &metadata,
                 sizeof(ModuleFrame),
                 ZMQ_SNDMORE);

        zmq_send(socket,
                 data.second,
                 MODULE_N_BYTES,
                 0);

        ring_buffer.release(data.first->buffer_slot_index);

        // TODO: Make real statistics, please.
        stats_counter++;

        if (data.first->n_recv_packets < JUNGFRAU_N_PACKETS_PER_FRAME) {
            n_missed_packets +=
                    JUNGFRAU_N_PACKETS_PER_FRAME - data.first->n_recv_packets;
        }

        if (last_pulse_id>0) {
            n_missed_frames += (pulse_id - last_pulse_id) - 1;
        }
        last_pulse_id = pulse_id;

        if (stats_counter == STATS_MODULO) {
            cout << "sf_buffer:device_name " << device_name;
            cout << " sf_buffer:pulse_id " << pulse_id;
            cout << " sf_buffer:n_missed_frames " << n_missed_frames;
            cout << " sf_buffer:n_missed_packets " << n_missed_packets;
            cout << endl;


            stats_counter = 0;
            n_missed_packets = 0;
            n_missed_frames = 0;
        }
    }
}
