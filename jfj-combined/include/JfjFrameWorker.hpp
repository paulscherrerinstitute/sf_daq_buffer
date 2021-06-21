#ifndef SF_DAQ_BUFFER_JFJ_UDPRECEIVER_HPP
#define SF_DAQ_BUFFER_JFJ_UDPRECEIVER_HPP

#include "PacketUdpReceiver.hpp"
#include "formats.hpp"
#include "buffer_config.hpp"
#include "PacketBuffer.hpp"
#include "jungfraujoch.hpp"

/** JungfrauJoch UDP receiver

    Wrapper class to capture frames from the UDP stream of the JungfrauJoch FPGA card.
    NOTE: This design will not scale well for higher frame rates...
**/
class JfjFrameWorker {
    PacketUdpReceiver m_udp_receiver;
    bool in_progress = false;
    uint64_t m_frame_index = 0;
    const uint64_t m_num_modules;
    const uint64_t m_num_packets;
    const uint64_t m_num_data_bytes;

    // PacketBuffer<jfjoch_packet_t, buffer_config::BUFFER_UDP_N_RECV_MSG> m_buffer;
    PacketBuffer<jfjoch_packet_t, 64> m_buffer;

    inline void init_frame(ModuleFrame& frame_metadata, const jfjoch_packet_t& c_packet);
    inline uint64_t process_packets(ModuleFrame& metadata, char* frame_buffer);

    std::function<void(uint64_t index, uint32_t module, char* ptr_data, ModuleFrame* ptr_meta)> f_push_callback;
public:
    JfjFrameUdpReceiver(const uint16_t port, std::function<void(uint64_t index, uint32_t module, char* ptr_data, ModuleFrame* ptr_meta)> callback);
    virtual ~JfjFrameUdpReceiver();
    std::generator<uint64_t> get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer);
    void run();
};

#endif //SF_DAQ_BUFFER_JFJ_UDPRECEIVER_HPP
