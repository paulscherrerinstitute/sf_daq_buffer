#include <cstring>
#include "BufferUdpReceiver.hpp"

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

inline void BufferUdpReceiver::init_frame (ModuleFrame& frame_metadata)
{
    frame_metadata.pulse_id = packet_buffer_.bunchid;
    frame_metadata.frame_index = packet_buffer_.framenum;
    frame_metadata.daq_rec = (uint64_t) packet_buffer_.debug;
    frame_metadata.module_id = (int64_t) source_id_;
}

void BufferUdpReceiver::get_frame_from_udp(
        ModuleFrame& metadata, char* frame_buffer)
{
    // Reset the metadata and frame buffer for the next frame.
    metadata.pulse_id = 0;
    metadata.n_received_packets = 0;
    memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);

    // The buffer contains a valid packet. Use it.
    if (packet_buffer_.bunchid != 0) {
        init_frame(metadata);

        size_t frame_buffer_offset =
                JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer_.packetnum;
        memcpy(
                (void*) (frame_buffer + frame_buffer_offset),
                packet_buffer_.data,
                JUNGFRAU_DATA_BYTES_PER_PACKET);

        metadata.n_received_packets++;
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
            return;
        }

        size_t frame_buffer_offset =
                JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer_.packetnum;
        memcpy(
                (void*) (frame_buffer + frame_buffer_offset),
                packet_buffer_.data,
                JUNGFRAU_DATA_BYTES_PER_PACKET);

        metadata.n_received_packets++;

        // Last frame packet received. Frame finished.
        if (packet_buffer_.packetnum == JUNGFRAU_N_PACKETS_PER_FRAME-1)
        {
            // Indicates that the packet has already been consumed.
            packet_buffer_.bunchid = 0;
            return;
        }
    }
}