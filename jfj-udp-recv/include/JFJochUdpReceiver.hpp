#ifndef SF_DAQ_BUFFER_JOCHUDPRECEIVER_HPP
#define SF_DAQ_BUFFER_JOCHUDPRECEIVER_HPP

#include <netinet/in.h>
#include "PacketUdpReceiver.hpp"
#include "formats.hpp"
#include "buffer_config.hpp"

class JFJochUdpReceiver {
    PacketUdpReceiver m_udp_receiver;

    // Incoming packet buffers
    jfjoch_packet_t m_packet_buffer[buffer_config::BUFFER_UDP_N_RECV_MSG];
    iovec           m_recv_buff_ptr[buffer_config::BUFFER_UDP_N_RECV_MSG];
    mmsghdr         m_msgs[buffer_config::BUFFER_UDP_N_RECV_MSG];
    sockaddr_in     m_sock_from[buffer_config::BUFFER_UDP_N_RECV_MSG];

    bool packet_buffer_loaded_ = false;
    int packet_buffer_n_packets_ = 0;
    int packet_buffer_offset_ = 0;

    inline void init_frame(ImageMetadata& frame_metadata, const int i_packet);
    inline void copy_packet_to_buffers(ImageMetadata& metadata, char* frame_buffer, const int i_packet);
    inline uint64_t m_process_packets(const int n_packets, ImageMetadata& metadata, char* frame_buffer);

public:
    JFJochUdpReceiver(const uint16_t port, const int module_id);
    virtual ~JFJochUdpReceiver();
    uint64_t get_frame_from_udp(ImageMetadata& metadata, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_JOCHUDPRECEIVER_HPP
