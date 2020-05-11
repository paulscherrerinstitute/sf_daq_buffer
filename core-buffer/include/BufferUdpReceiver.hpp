#ifndef SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP
#define SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP


#include "UdpReceiver.hpp"
#include "jungfrau.hpp"

class BufferUdpReceiver {
    const int source_id_;
    UdpReceiver udp_receiver_;

public:
    BufferUdpReceiver(const uint16_t port, const int source_id);
    virtual ~BufferUdpReceiver();
    void get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_BUFFERUDPRECEIVER_HPP
