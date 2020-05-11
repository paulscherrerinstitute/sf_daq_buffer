#ifndef SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP
#define SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP


#include "UdpReceiver.hpp"
#include "jungfrau.hpp"

class BufferUdpReceiver {
    const int source_id_;

    UdpReceiver udp_receiver_;
    jungfrau_packet packet_buffer_ = {};
    bool packet_buffer_loaded_ = false;

    inline void init_frame(ModuleFrame& frame_metadata);
    inline void copy_packet_to_buffers(
            ModuleFrame& metadata, char* frame_buffer);

public:
    BufferUdpReceiver(const uint16_t port, const int source_id);
    virtual ~BufferUdpReceiver();
    uint64_t get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP
