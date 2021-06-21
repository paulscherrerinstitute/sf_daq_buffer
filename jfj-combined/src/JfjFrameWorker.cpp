#include <cstring>
#include "JfjFrameWorker.hpp"

using namespace std;
using namespace buffer_config;



JfjFrameWorker::JfjFrameWorker(const uint16_t port, const uint32_t moduleID,
                               std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> callback):
        m_moduleID(moduleID), f_push_callback(callback) {
    m_udp_receiver.bind(port);
    m_state = "ON";
}


JfjFrameWorker::~JfjFrameWorker() {
    m_udp_receiver.disconnect();
}


inline uint64_t JfjFrameWorker::process_packets(BufferBinaryFormat& buffer){
    std::cout << "  Called process_packets()" << std::endl;

    while(!m_buffer.is_empty()){
        // Happens if the last packet from the previous frame gets lost.
        if (m_frame_index != m_buffer.peek_front().framenum) {
            m_frame_index = m_buffer.peek_front().framenum;
            if(this->in_progress){
                this->in_progress = false;
                return buffer.meta.pulse_id;
            }
        }

        // Otherwise pop the queue (and set current frame index)
        jfjoch_packet_t& c_packet = m_buffer.pop_front();
        m_frame_index = c_packet.framenum;
        this->in_progress = true;
        std::cout << "    pp: " << c_packet.packetnum << std::endl;

        // Always copy metadata (otherwise problem when 0th packet gets lost)
        buffer.meta.pulse_id = c_packet.bunchid;
        buffer.meta.frame_index = c_packet.framenum;
        buffer.meta.daq_rec = c_packet.debug;
        buffer.meta.module_id = m_moduleID;
        
        // Copy data to frame buffer
        size_t offset = JUNGFRAU_DATA_BYTES_PER_PACKET * c_packet.packetnum;
        memcpy( (void*) (buffer.data + offset), c_packet.data, JUNGFRAU_DATA_BYTES_PER_PACKET);
        buffer.meta.n_recv_packets++;

        // Last frame packet received. Frame finished.
        if (c_packet.packetnum == JF_N_PACKETS_PER_FRAME - 1){
            std::cout << "Finished pulse: " << buffer.meta.pulse_id << std::endl;
            this->in_progress = false;
            return buffer.meta.pulse_id;
        }
    }

    // We emptied the buffer.
    return 0;
}

uint64_t JfjFrameWorker::get_frame(BufferBinaryFormat& buffer){
    std::cout << "Called get_frame()" << std::endl;
    // Reset the metadata and frame buffer for the next frame. (really needed?)
    memset(&buffer, 0, sizeof(buffer));

    // Process leftover packages in the buffer
    if (!m_buffer.is_empty()) {
        auto pulse_id = process_packets(buffer);
        if (pulse_id != 0) { return pulse_id; }
    }


    while (true) {
        // Receive new packages (pass if none)...
        m_buffer.fill_from(m_udp_receiver);
        if (m_buffer.is_empty()) { continue; }

        // ... and process them
        auto pulse_id = process_packets(buffer);
        if (pulse_id != 0) { return pulse_id; }
    }
}

void JfjFrameWorker::run(){
    std::cout << "Running worker loop" << std::endl;
    // Might be better creating a structure for double buffering
    BufferBinaryFormat buffer;

    uint64_t pulse_id_previous = 0;
    uint64_t frame_index_previous = 0;

    try{
        m_state = "RUNNING";
        while (true) {
            // NOTE: Needs to be pipelined for really high frame rates
            auto pulse_id = get_frame(buffer);

            if(pulse_id>10){
                std::cout << "Pushing " << pulse_id << std::endl;
                f_push_callback(pulse_id, m_moduleID, buffer);
            }
        }
    } catch (const std::exception& ex) {
        std::cout << "Exception in worker loop: " << ex.what() << std::endl;
        m_state = "ERROR";
        throw;
    };

}

std::string JfjFrameWorker::print() const {
    std::string msg = "JungfrauFrameWorker #" + std::to_string(m_moduleID) + "\n"+
                        "State:\t" + m_state + "\n";
    return msg;
}


std::ostream& operator<<(std::ostream& os, const JfjFrameWorker& worker){
    os << worker.print() << std::endl;
    return os;
}


