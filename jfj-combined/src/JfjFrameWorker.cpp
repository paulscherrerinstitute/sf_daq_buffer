#include <sstream>

#include "JfjFrameWorker.hpp"


JfjFrameWorker::JfjFrameWorker(const uint16_t port, std::string moduleName, const uint32_t moduleID, std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> callback):
        m_moduleName(moduleName), m_moduleID(moduleID), m_moduleStats(moduleName, moduleID, 10.0), f_push_callback(callback) {
    m_udp_receiver.bind(port);
}


JfjFrameWorker::~JfjFrameWorker() {
    m_udp_receiver.disconnect();
}

/** Process Packets

    Drains the buffer either until it's empty or the current frame is finished.
    Has some optimizations and safety checks before segfaulting right away...
    TODO: Direct memcopy into FrameCache for more speed!                    **/
inline uint64_t JfjFrameWorker::process_packets(BufferBinaryFormat& buffer){

    while(!m_buffer.is_empty()){
        // Happens if the last packet from the previous frame gets lost.
        if (m_current_index != m_buffer.peek_front().bunchid) [[unlikely]] {
            m_current_index = m_buffer.peek_front().bunchid;
            if(this->in_progress){
                this->in_progress = false;
                return buffer.meta.pulse_id;
            }
        }

        // Otherwise pop the queue (and set current frame index)
        jfjoch_packet_t& c_packet = m_buffer.pop_front();

        // Sanity check: rather throw than segfault...
        if(c_packet.packetnum >= JF_N_PACKETS_PER_FRAME) [[unlikely]] {
            std::stringstream ss;
            ss << "Packet index '" << c_packet.packetnum "' is out of range of " << JF_N_PACKETS_PER_FRAME << std::endl;
            throw std::range_error(ss.str());
        }

        // Start new frame
        if(!this->in_progress) [[unlikely]] {
            m_current_index = c_packet.bunchid;
            this->in_progress = true;

            // Always copy metadata (otherwise problem when 0th packet gets lost)
            buffer.meta.pulse_id = c_packet.bunchid;
            buffer.meta.frame_index = c_packet.framenum;
            buffer.meta.daq_rec = c_packet.debug;
            buffer.meta.module_id = m_moduleID;
        }

        // Copy data to frame buffer
        size_t offset = JUNGFRAU_DATA_BYTES_PER_PACKET * c_packet.packetnum;
        std::memcpy( (void*) (buffer.data + offset), c_packet.data, JUNGFRAU_DATA_BYTES_PER_PACKET);
        buffer.meta.n_recv_packets++;

        // Last frame packet received. Frame finished.
        if (c_packet.packetnum == JF_N_PACKETS_PER_FRAME - 1) [[unlikely]] {
            this->in_progress = false;
            return buffer.meta.pulse_id;
        }
    }

    // We emptied the buffer.
    return 0;
}

uint64_t JfjFrameWorker::get_frame(BufferBinaryFormat& buffer){
    // Reset the metadata and frame buffer for the next frame
    std::memset(&buffer, 0, sizeof(buffer));
    uint64_t pulse_id = 0;

    // Hehehehe... do-while loop!
     do {
        // First make sure the buffer is drained of leftovers
        pulse_id = process_packets(buffer);
        if (pulse_id != 0) [[likely]] { return pulse_id; }

        // Then try to refill buffer...
        m_buffer.fill_from(m_udp_receiver);
    } while (true);
}


void JfjFrameWorker::run(){
    std::cout << "Running worker loop" << std::endl;
    // Might be better creating a structure for double buffering
    BufferBinaryFormat buffer;

    try{
        while (true) {
            // NOTE: Needs to be pipelined for really high frame rates
            auto pulse_id = get_frame(buffer);
            m_moduleStats.record_stats(buffer.meta, true);

            if(pulse_id>10) [[likely]] {
                f_push_callback(pulse_id, m_moduleID, buffer);
            }
        }
    } catch (const std::exception& ex) {
        std::cout << "Exception in worker loop: " << ex.what() << std::endl;
        throw;
    };
}
