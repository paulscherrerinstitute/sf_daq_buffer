#ifndef SF_DAQ_BUFFER_JFJ_FRAMEWORKER_HPP
#define SF_DAQ_BUFFER_JFJ_FRAMEWORKER_HPP

#include <iostream>
#include <functional>
#include "../../core-buffer/include/formats.hpp"
#include "PacketUdpReceiver.hpp"
#include "PacketBuffer.hpp"

/** JungfrauJoch UDP receiver

    Wrapper class to capture frames from the UDP stream of the JungfrauJoch FPGA card.
    NOTE: This design will not scale well for higher frame rates...
**/
class JfjFrameWorker {
    std::string m_state = "INIT";
    PacketUdpReceiver m_udp_receiver;
    bool in_progress = false;
    uint64_t m_frame_index = 0;
    const uint64_t m_moduleID;
    const uint64_t m_num_packets;
    const uint64_t m_num_data_bytes;

    PacketBuffer<jfjoch_packet_t, 64> m_buffer;

    inline uint64_t process_packets(BufferBinaryFormat& buffer);
    uint64_t get_frame(BufferBinaryFormat& buffer);

    std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> f_push_callback;
public:
    JfjFrameWorker(const uint16_t port, const uint32_t moduleID,
                   std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> callback);
    virtual ~JfjFrameWorker();
    std::string print() const;
    void run();
};


std::ostream& operator<<(std::ostream& os, const JfjFrameWorker& worker){
    os << worker.print() << std::endl;
    return os;
}


#endif //SF_DAQ_BUFFER_JFJ_FRAMEWORKER_HPP
