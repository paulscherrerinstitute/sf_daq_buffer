#include <cstring>
#include <jungfraujoch.hpp>
#include "JFJochUdpReceiver.hpp"

using namespace std;
using namespace buffer_config;

JFJochUdpReceiver::JFJochUdpReceiver(const uint16_t port, const int module_id) : module_id_(module_id){
    udp_receiver_.bind(port);

    for (int i = 0; i < BUFFER_UDP_N_RECV_MSG; i++) {
        m_recv_buff_ptr[i].iov_base = (void*) &(m_packet_buffer[i]);
        m_recv_buff_ptr[i].iov_len = sizeof(jfjoch_packet_t);

        msgs_[i].msg_hdr.msg_iov = &m_recv_buff_ptr[i];
        msgs_[i].msg_hdr.msg_iovlen = 1;
        msgs_[i].msg_hdr.msg_name = &m_sock_from[i];
        msgs_[i].msg_hdr.msg_namelen = sizeof(sockaddr_in);
    }
}

JFJochUdpReceiver::~JFJochUdpReceiver() {
    m_udp_receiver.disconnect();
}

inline void JFJochUdpReceiver::init_frame(ImageMetadata& image_metadata, const int i_packet) {
    image_metadata.pulse_id = m_packet_buffer[i_packet].bunchid;
    image_metadata.frame_index = m_packet_buffer[i_packet].framenum;
    image_metadata.daq_rec = m_packet_buffer[i_packet].debug;
    image_metadata.is_good_image = 0;
}

inline void JFJochUdpReceiver::copy_packet_to_buffers(ImageMetadata& metadata, char* frame_buffer, const int idx_packet){

    size_t buffer_offset = JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer_[idx_packet].packetnum;

    memcpy((void*) (frame_buffer + buffer_offset), m_packet_buffer[idx_packet].data, JUNGFRAU_DATA_BYTES_PER_PACKET);

    metadata.n_recv_packets++;
}



/** Copy the contents of the packet buffer into a single assembled image
    NOTE: In the jungfrau_packet, framenum is the trigger number
    NOTE: Even partial frames are valid
**/
inline uint64_t JFJochUdpReceiver::m_process_packets(const int start_offset, ImageMetadata& metadata, char* frame_buffer){

    for (int i_packet=start_offset; i_packet < packet_buffer_n_packets_; i_packet++) {

        // First packet for this frame (sucks if this one is missed)
        if (metadata.pulse_id == 0) {
            init_frame(metadata, i_packet);
        }
        // Unexpected jump (if the last packet from the previous frame got lost)
        if (metadata.frame_index != m_packet_buffer[i_packet].framenum) {
            // Save queue status (lazy fifo queue)
            m_packet_buffer_loaded_ = true;
            m_packet_buffer_offset_ = i_packet;
            // Even partial frames are valid?
            return metadata.pulse_id;
        }

        copy_packet_to_buffers(metadata, frame_buffer, i_packet);

        // Last frame packet received (frame finished)
        if (packet_buffer_[i_packet].packetnum == JFJ_N_PACKETS_PER_FRAME - 1) {
            // Buffer is loaded only if this is not the last message.
            if (i_packet+1 != packet_buffer_n_packets_) {
                // Continue on next packet
                m_packet_buffer_loaded = true;
                m_packet_buffer_offset = i_packet + 1;

            // If i_packet is the last packet the buffer is empty.
            } else {
                m_packet_buffer_loaded = true;
                m_packet_buffer_offset = 0;
            }

            return metadata.pulse_id;
        }
    }
    // We emptied the buffer.
    m_packet_buffer_loaded = false;
    m_packet_buffer_offset = 0;

    return 0;
}

uint64_t JFJochUdpReceiver::get_frame_from_udp(ImageMetadata& metadata, char* frame_buffer){
    // Reset the metadata and frame buffer for the next frame.
    metadata.pulse_id = 0;
    metadata.n_recv_packets = 0;
    memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);

    // Happens when last packet from previous frame was missed.
    if (packet_buffer_loaded_) {
        auto pulse_id = m_process_packets(packet_buffer_offset_, metadata, frame_buffer);
        if (pulse_id != 0) { return pulse_id; }
    }

    // Otherwise read a new one
    while (true) {
        packet_buffer_n_packets_ = udp_receiver_.receive_many(msgs_, BUFFER_UDP_N_RECV_MSG);

        if (packet_buffer_n_packets_ > 0) {
            auto pulse_id = m_process_packets(0, metadata, frame_buffer);
            if (pulse_id != 0) { return pulse_id; }
        }
    }
}
