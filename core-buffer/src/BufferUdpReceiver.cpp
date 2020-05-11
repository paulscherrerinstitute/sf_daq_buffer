#include <cstring>
#include <jungfrau.hpp>
#include "BufferUdpReceiver.hpp"
#include <iostream>

using namespace std;

BufferUdpReceiver::BufferUdpReceiver(
        const uint16_t port,
        const int source_id) :
            source_id_(source_id)
{
    udp_receiver_.bind(port);
}

BufferUdpReceiver::~BufferUdpReceiver() {
    udp_receiver_.disconnect();
}

inline void BufferUdpReceiver::init_frame(ModuleFrame& frame_metadata)
{
    frame_metadata.pulse_id = packet_buffer_.bunchid;
    frame_metadata.frame_index = packet_buffer_.framenum;
    frame_metadata.daq_rec = (uint64_t) packet_buffer_.debug;
    frame_metadata.module_id = (int64_t) source_id_;
}

inline void BufferUdpReceiver::copy_packet_to_buffers(
        ModuleFrame& metadata, char* frame_buffer)
{
    size_t frame_buffer_offset =
            JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer_.packetnum;
    memcpy(
            (void*) (frame_buffer + frame_buffer_offset),
            packet_buffer_.data,
            JUNGFRAU_DATA_BYTES_PER_PACKET);

    metadata.n_received_packets++;
}

uint64_t BufferUdpReceiver::get_frame_from_udp(
        ModuleFrame& metadata, char* frame_buffer)
{
    // Reset the metadata and frame buffer for the next frame.
    metadata.pulse_id = 0;
    metadata.n_received_packets = 0;
    memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);

    // Happens when last packet from previous frame was missed.
    if (packet_buffer_loaded_) {
        packet_buffer_loaded_ = false;

        init_frame(metadata);
        copy_packet_to_buffers(metadata, frame_buffer);
    }

    while (true) {

        if (!udp_receiver_.receive(
                &packet_buffer_,
                JUNGFRAU_BYTES_PER_PACKET)) {
            continue;
        }

        // First packet for this frame.
        if (metadata.pulse_id == 0) {
            init_frame(metadata);

        // Happens if the last packet from the previous frame gets lost.
        } else if (metadata.pulse_id != packet_buffer_.bunchid) {
            cout << "source_id " << source_id_ << " missing last packet";
            cout << " pulse_id " << metadata.pulse_id << endl;

            packet_buffer_loaded_ = true;

            return metadata.pulse_id;
        }

        copy_packet_to_buffers(metadata, frame_buffer);

        // Last frame packet received. Frame finished.
        if (packet_buffer_.packetnum == JUNGFRAU_N_PACKETS_PER_FRAME-1)
        {
            return metadata.pulse_id;
        }
    }
}