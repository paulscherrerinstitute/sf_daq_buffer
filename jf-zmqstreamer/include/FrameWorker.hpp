#ifndef SF_DAQ_BUFFER_JFJ_FRAMEWORKER_HPP
#define SF_DAQ_BUFFER_JFJ_FRAMEWORKER_HPP

#include <iostream>
#include <cstring>
#include <functional>

#include "../../core-buffer/include/formats.hpp"
#include "PacketUdpReceiver.hpp"
#include "PacketBuffer.hpp"
#include "FrameStats.hpp"

/** Jungfrau UDP receiver

    Capture UDP data stream from Jungfrau(Joch) FPGA card.
    NOTE: This design will not scale well for higher frame rates...
    TODO: Direct copy into FrameCache buffer (saves a memcopy)
**/
class FrameWorker {
    const std::string m_moduleName;
    const uint64_t m_moduleID;

    // UDP and statistics interfaces
    FrameStats m_moduleStats;
    PacketUdpReceiver m_udp_receiver;

    // Buffer and helper structures
    bool in_progress = false;
    uint64_t m_current_index = 0;
    PacketBuffer<jungfrau_packet_t, 64> m_buffer;

    // Buffer processing
    inline uint64_t process_packets(BufferBinaryFormat& buffer);
    uint64_t get_frame(BufferBinaryFormat& buffer);

    std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> f_push_callback;
public:
    FrameWorker(const uint16_t port, std::string moduleName, const uint32_t moduleID, std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> callback);
    virtual ~FrameWorker();
    void run();
    
    // Copy semantics : OFF
    FrameWorker(FrameWorker const &) = delete;
    FrameWorker& operator=(FrameWorker const &) = delete;
};

#endif //SF_DAQ_BUFFER_JFJ_FRAMEWORKER_HPP
