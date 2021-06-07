#ifndef SF_DAQ_BUFFER_JOCHUDPRECEIVER_HPP
#define SF_DAQ_BUFFER_JOCHUDPRECEIVER_HPP

#include <netinet/in.h>
#include "PacketUdpReceiver.hpp"
#include "formats.hpp"
#include "buffer_config.hpp"
#include "PacketBuffer.hpp"
#include "jungfraujoch.hpp"

/** JungfrauJoch UDP receiver

    Wrapper class to capture frames from the UDP stream of the JungfrauJoch FPGA card.
    NOTE: This design will not scale well for higher frame rates...
**/
class JfjFrameUdpReceiver {
    PacketUdpReceiver m_udp_receiver;
    uint64_t m_frame_index;

    PacketBuffer<jfjoch_packet_t, buffer_config::BUFFER_UDP_N_RECV_MSG> m_buffer;

    inline void init_frame(ModuleFrame& frame_metadata, const jfjoch_packet_t& c_packet);
    inline uint64_t process_packets(ModuleFrame& metadata, char* frame_buffer);

public:
    JfjFrameUdpReceiver(const uint16_t port);
    virtual ~JfjFrameUdpReceiver();
    uint64_t get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_JOCHUDPRECEIVER_HPP
