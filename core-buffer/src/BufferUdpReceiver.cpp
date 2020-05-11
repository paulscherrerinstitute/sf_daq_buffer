#include "BufferUdpReceiver.hpp"

BufferUdpReceiver::BufferUdpReceiver(int source_id) :
        source_id_(source_id)
{

}

void BufferUdpReceiver::bind(const uint16_t port)
{
    udp_receiver_.bind(port);
}

inline void BufferUdpReceiver::init_frame (
        ModuleFrame& frame_metadata,
        jungfrau_packet& packet_buffer,
        uint64_t source_id)
{
    frame_metadata.pulse_id = packet_buffer.bunchid;
    frame_metadata.frame_index = packet_buffer.framenum;
    frame_metadata.daq_rec = (uint64_t)packet_buffer.debug;
    frame_metadata.module_id = source_id;
}

void BufferUdpReceiver::get_frame_from_udp(
        ModuleFrame& metadata, char* frame_buffer)
{
    static jungfrau_packet packet_buffer = {};

    // Reset the metadata and frame buffer for the next frame.
    metadata.pulse_id = 0;
    metadata.n_received_packets = 0;
    memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);

    // The buffer contains a valid packet. Use it.
    if (packet_buffer.bunchid != 0) {
        init_frame(metadata, packet_buffer, source_id);

        size_t frame_buffer_offset =
                JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer.packetnum;
        memcpy(
                (void*) (frame_buffer + frame_buffer_offset),
                packet_buffer.data,
                JUNGFRAU_DATA_BYTES_PER_PACKET);

        metadata.n_received_packets++;
    }

    while (true) {

        if (!udp_receiver.receive(
                &packet_buffer,
                JUNGFRAU_BYTES_PER_PACKET)) {
            continue;
        }

        // First packet for this frame.
        if (metadata.pulse_id == 0) {
            init_frame(metadata, packet_buffer, source_id);

            // Happens if the last packet from the previous frame gets lost.
        } else if (metadata.pulse_id != packet_buffer.bunchid) {
            return;
        }

        size_t frame_buffer_offset =
                JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer.packetnum;
        memcpy(
                (void*) (frame_buffer + frame_buffer_offset),
                packet_buffer.data,
                JUNGFRAU_DATA_BYTES_PER_PACKET);

        metadata.n_received_packets++;

        // Last frame packet received. Frame finished.
        if (packet_buffer.packetnum == JUNGFRAU_N_PACKETS_PER_FRAME-1)
        {
            // Indicates that the packet has already been consumed.
            packet_buffer.bunchid = 0;
            return;
        }
    }
}