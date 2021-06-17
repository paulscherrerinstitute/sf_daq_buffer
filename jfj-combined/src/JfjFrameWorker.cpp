#include <cstring>
#include "JfjFrameWorker.hpp"

using namespace std;
using namespace buffer_config;



JfjFrameWorker::JfjFrameWorker(const uint16_t port, std::function<void(uint64_t index, uint32_t module, char* ptr_data, ModuleFrame* ptr_meta)> callback):
        m_num_modules(n_modules), m_num_packets(n_modules*JFJOCH_N_PACKETS_PER_MODULE),
        m_num_data_bytes(n_modules*JFJOCH_DATA_BYTES_PER_MODULE), f_push_callback(callback) {
    m_udp_receiver.bind(port);
}

JfjFrameWorker::~JfjFrameWorker() {
    m_udp_receiver.disconnect();
}

inline void JfjFrameWorker::init_frame(ModuleFrame& metadata, const jfjoch_packet_t& c_packet) {
    metadata.pulse_id = c_packet.bunchid;
    metadata.frame_index = c_packet.framenum;
    metadata.daq_rec = (uint64_t) c_packet.debug;
    metadata.module_id = (int64_t) 0;
}

inline uint64_t JfjFrameWorker::process_packets(ModuleFrame& metadata, char* frame_buffer){

    while(!m_buffer.is_empty()){
        // Happens if the last packet from the previous frame gets lost.
        if (m_frame_index != m_buffer.peek_front().framenum) {
            m_frame_index = m_buffer.peek_front().framenum;
            if(this->in_progress){
                this->in_progress = false;
                return metadata.pulse_id;
            }
        }

        // Otherwise pop the queue (and set current frame index)
        jfjoch_packet_t& c_packet = m_buffer.pop_front();
        m_frame_index = c_packet.framenum;
        this->in_progress = true;

        // Always copy metadata (otherwise problem when 0th packet gets lost)
        this->init_frame(metadata, c_packet);

        // Copy data to frame buffer
        size_t offset = JFJOCH_DATA_BYTES_PER_PACKET * c_packet.packetnum;
        memcpy( (void*) (frame_buffer + offset), c_packet.data, JFJOCH_DATA_BYTES_PER_PACKET);
        metadata.n_recv_packets++;

        // Last frame packet received. Frame finished.
        if (c_packet.packetnum == m_num_packets - 1){
            this->in_progress = false;
            return metadata.pulse_id;
        }
    }

    // We emptied the buffer.
   // m_buffer.reset();
    return 0;
}

uint64_t JfjFrameWorker::get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer){
    // Reset the metadata and frame buffer for the next frame. (really needed?)
    metadata.pulse_id = 0;
    metadata.n_recv_packets = 0;
    memset(frame_buffer, 0, m_num_data_bytes);


    // Process leftover packages in the buffer
    if (!m_buffer.is_empty()) {
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { return pulse_id; }
    }


    while (true) {
        // Receive new packages (pass if none)...
        m_buffer.fill_from(m_udp_receiver);
        if (m_buffer.is_empty()) { continue; }

        // ... and process them
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { return pulse_id; }
    }
}





std::generator<uint64_t> JfjFrameWorker::get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer){
    // Reset the metadata and frame buffer for the next frame. (really needed?)
    metadata.pulse_id = 0;
    metadata.n_recv_packets = 0;
    memset(frame_buffer, 0, m_num_data_bytes);


    // Process leftover packages in the buffer
    if (!m_buffer.is_empty()) {
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { co_yield pulse_id; }
    }


    while (true) {
        // Receive new packages (pass if none)...
        m_buffer.fill_from(m_udp_receiver);
        if (m_buffer.is_empty()) { continue; }

        // ... and process them
        auto pulse_id = process_packets(metadata, frame_buffer);
        if (pulse_id != 0) { co_yield pulse_id; }
    }
}

void JfjFrameWorker::run(){


    // Might be better creating a structure for double buffering
    ModuleFrame frameMeta;
    char* dataBuffer = new char[JFJOCH_DATA_BYTES_PER_MODULE];

    uint64_t pulse_id_previous = 0;
    uint64_t frame_index_previous = 0;


    while (true) {
        // NOTE: Needs to be pipelined for really high frame rates
        auto pulse_id = co_await get_frame_from_udp(&frameMeta, dataBuffer);

        if(pulse_id>1000){
            f_push_callback(pulse_id, m_moduleID, dataBuffer, frameMeta);
        }
    }

    delete[] dataBuffer;
}













