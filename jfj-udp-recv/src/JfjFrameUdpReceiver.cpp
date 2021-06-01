#include <cstring>
#include <jungfrau.hpp>
#include "JfjFrameUdpReceiver.hpp"

using namespace std;
using namespace buffer_config;

JfjFrameUdpReceiver::JfjFrameUdpReceiver(const uint16_t port) {
    udp_receiver_.bind(port);
}

JfjFrameUdpReceiver::~JfjFrameUdpReceiver() {
    udp_receiver_.disconnect();
}

inline void JfjFrameUdpReceiver::init_frame(ImageMetadata& frame_metadata, const jfjoch_packet_t& c_packet) {
        frame_metadata.pulse_id = c_packet.timestamp;
        frame_metadata.frame_index = c_packet.framenum;
        frame_metadata.daq_rec = (uint32_t) c_packet.debug;
        frame_metadata.is_good_image = (int32_t) true;
}

inline uint64_t JfjFrameUdpReceiver::process_packets(ImageMetadata& metadata, char* frame_buffer){

    while(!m_buffer.is_empty()){
        // Happens if the last packet from the previous frame gets lost.
        if (m_frame_index != m_buffer.peek_front().framenum) {
            m_frame_index = m_buffer.peek_front().framenum;
            frame_metadata.is_good_image = (int32_t) false;
            return metadata.pulse_id;
        }

        // Otherwise pop the queue (and set current frame index)
        jfjoch_packet_t& c_packet = m_buffer.pop_front();
        m_frame_index = c_packet.framenum;

        // Always copy metadata (otherwise problem when 0th packet gets lost)
        this->init_frame(metadata, c_packet);

        // Copy data to frame buffer
        size_t offset = JUNGFRAU_DATA_BYTES_PER_PACKET * c_packet.packetnum;
        memcpy( (void*) (frame_buffer + offset), c_packet.data, JUNGFRAU_DATA_BYTES_PER_PACKET);
        metadata.n_recv_packets++;

        // Last frame packet received. Frame finished.
        if (c_packet.packetnum == JFJ_N_PACKETS_PER_FRAME - 1){
            return metadata.pulse_id;
        }
    }

    // We emptied the buffer.
    m_buffer.reset();
    return 0;
}

uint64_t JfjFrameUdpReceiver::get_frame_from_udp(ImageMetadata& metadata, char* frame_buffer){
    // Reset the metadata and frame buffer for the next frame.
    metadata.pulse_id = 0;
    metadata.n_recv_packets = 0;
    memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);

    // Process leftover packages in the buffer
    if (!m_buffer.is_empty()) {
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { return pulse_id; }
    }

    while (true) {
        // Receive new packages (pass if none)...
        m_buffer.fill_from(m_udp_receiver);
        if (m_buffer.is_empty()) { continue; }

        // ... and process them
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { return pulse_id; }
    }
}
