#include <cstring>
#include "JfjFrameUdpReceiver.hpp"

using namespace std;
using namespace buffer_config;

std::ostream &operator<<(std::ostream &os, jfjoch_packet_t const &packet) { 
    os << "Frame number: " << packet.framenum << std::endl;
    os << "Packet number: " << packet.packetnum << std::endl;
    os << "Bunch id: " << packet.bunchid << std::endl;
    os << std::endl;
    return os;
}

JfjFrameUdpReceiver::JfjFrameUdpReceiver(const uint16_t port) {
    m_udp_receiver.bind(port);
}

JfjFrameUdpReceiver::~JfjFrameUdpReceiver() {
    m_udp_receiver.disconnect();
}

inline void JfjFrameUdpReceiver::init_frame(ModuleFrame& metadata, const jfjoch_packet_t& c_packet) {
    // std::cout << c_packet;
      
    metadata.pulse_id = c_packet.bunchid;
    metadata.frame_index = c_packet.framenum;
    metadata.daq_rec = (uint64_t) c_packet.debug;
    metadata.module_id = (int64_t) 0;
}

inline uint64_t JfjFrameUdpReceiver::process_packets(ModuleFrame& metadata, char* frame_buffer){

    while(!m_buffer.is_empty()){
        // Happens if the last packet from the previous frame gets lost.
        if (m_frame_index != m_buffer.peek_front().framenum) {
            m_frame_index = m_buffer.peek_front().framenum;
            std::cout << "Peeked pulse: " << metadata.pulse_id << std::endl;
            return metadata.pulse_id;
        }

        // Otherwise pop the queue (and set current frame index)
        jfjoch_packet_t& c_packet = m_buffer.pop_front();
        m_frame_index = c_packet.framenum;

        // Always copy metadata (otherwise problem when 0th packet gets lost)
        this->init_frame(metadata, c_packet);

        // Copy data to frame buffer
        size_t offset = JFJOCH_DATA_BYTES_PER_PACKET * c_packet.packetnum;
        memcpy( (void*) (frame_buffer + offset), c_packet.data, JFJOCH_DATA_BYTES_PER_PACKET);
        metadata.n_recv_packets++;

        // Last frame packet received. Frame finished.
        if (c_packet.packetnum == JFJOCH_N_PACKETS_PER_FRAME - 1){
            return metadata.pulse_id;
        }
    }

    // We emptied the buffer.
   // m_buffer.reset();
    return 0;
}

uint64_t JfjFrameUdpReceiver::get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer){
    // Reset the metadata and frame buffer for the next frame. (really needed?)
    metadata.pulse_id = 0;
    metadata.n_recv_packets = 0;
    memset(frame_buffer, 0, JFJOCH_DATA_BYTES_PER_FRAME);
    // Process leftover packages in the buffer
    if (!m_buffer.is_empty()) {
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { return pulse_id; }
    }

    while (true) {
        // Receive new packages (pass if none)...
        m_buffer.reset();
        m_buffer.fill_from(m_udp_receiver);      
        if (m_buffer.is_empty()) { continue; }

        // ... and process them
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { return pulse_id; }
    }
}
