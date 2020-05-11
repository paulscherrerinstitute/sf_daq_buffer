#include <iostream>
#include <stdexcept>
#include <RingBuffer.hpp>
#include <BufferH5Writer.hpp>
#include <FastQueue.hpp>
#include <UdpReceiver.hpp>
#include "zmq.h"
#include "buffer_config.hpp"
#include "jungfrau.hpp"


using namespace std;
using namespace core_buffer;

inline void init_frame (
        ModuleFrame* frame_metadata,
        jungfrau_packet& packet_buffer,
        uint64_t source_id)
{
    frame_metadata->pulse_id = packet_buffer.bunchid;
    frame_metadata->frame_index = packet_buffer.framenum;
    frame_metadata->daq_rec = (uint64_t)packet_buffer.debug;
    frame_metadata->module_id = source_id;
}

void save_and_send(
        BufferH5Writer& writer,
        void* socket,
        ModuleFrame *metadata,
        char *data)
{
    // Write to file.
    writer.set_pulse_id(metadata->pulse_id);
    writer.write(metadata, data);

    // Live stream.
    zmq_send(socket, metadata, sizeof(ModuleFrame), ZMQ_SNDMORE);
    zmq_send(socket, data, MODULE_N_BYTES, 0);
}

int main (int argc, char *argv[]) {
    if (argc != 5) {
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
    int source_id = atoi(argv[4]);

    stringstream ipc_stream;
    ipc_stream << "ipc:///tmp/sf-live-" << source_id;
    const auto ipc_address = ipc_stream.str();

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUB);

    const int sndhwm = BUFFER_ZMQ_SNDHWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(strerror (errno));

    const int linger_ms = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(strerror (errno));

    if (zmq_bind(socket, ipc_address.c_str()) != 0)
        throw runtime_error(strerror (errno));

    uint64_t stats_counter(0);
    uint64_t n_missed_packets = 0;
    uint64_t n_missed_frames = 0;
    uint64_t n_corrupted_frames = 0;
    uint64_t last_pulse_id = 0;

    UdpReceiver udp_receiver;
    udp_receiver.bind(udp_port);

    BufferH5Writer writer(device_name, root_folder);

    jungfrau_packet packet_buffer;
    ModuleFrame* metadata;
    auto frame_buffer = new char[MODULE_N_BYTES * JUNGFRAU_N_MODULES];

    while (true) {

        if (!udp_receiver.receive(
                &packet_buffer,
                JUNGFRAU_BYTES_PER_PACKET)) {
            continue;
        }

        // First packet for this frame.
        if (metadata->pulse_id == 0) {
            init_frame(metadata, packet_buffer, source_id);

            // Happens if the last packet from the previous frame gets lost.
        } else if (metadata->pulse_id != packet_buffer.bunchid) {
            save_and_send(writer, socket, metadata, frame_buffer);

            metadata->pulse_id = 0;
            metadata->n_received_packets = 0;
            memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);

            init_frame(metadata, packet_buffer, source_id);
        }

        size_t frame_buffer_offset =
                JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer.packetnum;

        memcpy(
                (void*) (frame_buffer + frame_buffer_offset),
                packet_buffer.data,
                JUNGFRAU_DATA_BYTES_PER_PACKET);

        metadata->n_received_packets++;

        // Last frame packet received. Frame finished.
        if (packet_buffer.packetnum == JUNGFRAU_N_PACKETS_PER_FRAME-1)
        {
            save_and_send(writer, socket, metadata, frame_buffer);
            metadata->pulse_id = 0;
            metadata->n_received_packets = 0;
            memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);
        }



        // TODO: Make real statistics, please.
        auto pulse_id = metadata->pulse_id;
        stats_counter++;

        if (metadata->n_received_packets < JUNGFRAU_N_PACKETS_PER_FRAME) {
            n_missed_packets +=
                    JUNGFRAU_N_PACKETS_PER_FRAME - metadata->n_received_packets;
            n_corrupted_frames++;
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
            cout << " sf_buffer:n_corrupted_frames " << n_corrupted_frames;
            cout << endl;

            stats_counter = 0;
            n_missed_packets = 0;
            n_corrupted_frames = 0;
            n_missed_frames = 0;
        }
    }
}
