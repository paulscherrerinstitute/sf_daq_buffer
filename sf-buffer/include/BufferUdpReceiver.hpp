#ifndef SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP
#define SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP

#include <netinet/in.h>
#include "UdpReceiver.hpp"
#include "formats.hpp"
#include "buffer_config.hpp"

class BufferUdpReceiver {
    const int source_id_;

    UdpReceiver udp_receiver_;

    jungfrau_packet packet_buffer_[core_buffer::BUFFER_UDP_N_RECV_MSG];
    iovec recv_buff_ptr_[core_buffer::BUFFER_UDP_N_RECV_MSG];
    mmsghdr msgs_[core_buffer::BUFFER_UDP_N_RECV_MSG];
    sockaddr_in sock_from_[core_buffer::BUFFER_UDP_N_RECV_MSG];

    bool packet_buffer_loaded_ = false;
    int packet_buffer_n_packets_ = 0;
    int packet_buffer_offset_ = 0;

    inline void init_frame(ModuleFrame& frame_metadata, const int i_packet);
    inline void copy_packet_to_buffers(
            ModuleFrame& metadata, char* frame_buffer, const int i_packet);
    inline uint64_t process_packets(
            const int n_packets, ModuleFrame& metadata, char* frame_buffer);

public:
    BufferUdpReceiver(const uint16_t port, const int source_id);
    virtual ~BufferUdpReceiver();
    uint64_t get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP
