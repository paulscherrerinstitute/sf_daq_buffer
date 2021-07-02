#include <cstring>
#include "FrameUdpReceiver.hpp"
#include <ostream>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include "date.h"

using namespace std;
using namespace buffer_config;

FrameUdpReceiver::FrameUdpReceiver(
        const int module_id,
        const uint16_t port,
        const int n_modules,
        const int n_submodules,
        const int bit_depth):
            module_id_(module_id),
            bit_depth_(bit_depth),
            n_packets_per_frame_(bit_depth_ * MODULE_N_PIXELS / 8 / DATA_BYTES_PER_PACKET / n_modules * n_submodules),
            data_bytes_per_frame_(n_packets_per_frame_ * DATA_BYTES_PER_PACKET)
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [FrameUdpReceiver::FrameUdpReceiver] :";
        cout << " Details of FrameUdpReceiver:";
        cout << "module_id: " << module_id_;
        cout << " || port: " << port;
        cout << " || bit_depth : " << bit_depth_;
        cout << " || n_packets_per_frame_ : " << n_packets_per_frame_;
        cout << " || data_bytes_per_frame_: " << data_bytes_per_frame_ << " !!";
        cout << endl;
    #endif
    udp_receiver_.bind(port);
    for (int i = 0; i < BUFFER_UDP_N_RECV_MSG; i++) {
        recv_buff_ptr_[i].iov_base = (void*) &(packet_buffer_[i]);
        recv_buff_ptr_[i].iov_len = sizeof(det_packet);

        msgs_[i].msg_hdr.msg_iov = &recv_buff_ptr_[i];
        msgs_[i].msg_hdr.msg_iovlen = 1;
        msgs_[i].msg_hdr.msg_name = &sock_from_[i];
        msgs_[i].msg_hdr.msg_namelen = sizeof(sockaddr_in);
    }
}

FrameUdpReceiver::~FrameUdpReceiver() {
    udp_receiver_.disconnect();
}

inline void FrameUdpReceiver::init_frame(
        ModuleFrame& frame_metadata, const int i_packet)
{
    // Eiger has no pulse_id, frame number instead
    frame_metadata.pulse_id = packet_buffer_[i_packet].framenum;
    frame_metadata.frame_index = packet_buffer_[i_packet].framenum;
    frame_metadata.daq_rec = (uint64_t) packet_buffer_[i_packet].debug;
    frame_metadata.module_id = (int64_t) module_id_;
    
    frame_metadata.bit_depth = (int16_t) bit_depth_;	
    frame_metadata.row = (int16_t) packet_buffer_[i_packet].row;
    frame_metadata.column = (int16_t) packet_buffer_[i_packet].column;

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [FrameUdpReceiver::init_frame] :";
        cout << "module_id: " << module_id_;
        cout << " || row: " << frame_metadata.row;
        cout << " || column: " << frame_metadata.column;
        cout << " || pulse_id: " << frame_metadata.pulse_id;
        cout << " || frame_index: " << frame_metadata.frame_index;
        cout << endl;
    #endif

}

inline void FrameUdpReceiver::copy_packet_to_buffers(
        ModuleFrame& metadata, char* frame_buffer, const int i_packet)
{
    size_t frame_buffer_offset =
            DATA_BYTES_PER_PACKET * packet_buffer_[i_packet].packetnum;
    memcpy(
            (void*) (frame_buffer + frame_buffer_offset),
            packet_buffer_[i_packet].data,
            DATA_BYTES_PER_PACKET);
    
    metadata.n_recv_packets++;
    // cout << "[ frame" << metadata.frame_index << "] NUMBER OF RECV PACKETS : " << metadata.n_recv_packets ;
}

inline uint64_t FrameUdpReceiver::process_packets(
        const int start_offset,
        ModuleFrame& metadata,
        char* frame_buffer)
{
    for (int i_packet=start_offset;
         i_packet < packet_buffer_n_packets_;
         i_packet++) {

        // First packet for this frame.
        if (metadata.pulse_id == 0) {
            init_frame(metadata, i_packet);

        // Happens if the last packet from the previous frame gets lost.
        // In the jungfrau_packet, framenum is the trigger number (how many triggers from detector power-on) happened
        } else if (metadata.frame_index != packet_buffer_[i_packet].framenum) {
            packet_buffer_loaded_ = true;
            // Continue on this packet.
            packet_buffer_offset_ = i_packet;

            return metadata.frame_index;
        }

        copy_packet_to_buffers(metadata, frame_buffer, i_packet);
        
        // Last frame packet received. Frame finished.
        if (packet_buffer_[i_packet].packetnum ==
            n_packets_per_frame_ - 1)
        {
            #ifdef DEBUG_OUTPUT
                using namespace date;
                cout << " [" << std::chrono::system_clock::now();
                cout << "] [FrameUdpReceiver::process_packets] :";
                cout << " Frame " << metadata.frame_index << " || ";
                cout << packet_buffer_[i_packet].packetnum << " packets received.";
                cout << " PULSE ID "<<  metadata.pulse_id;
                cout << endl;
            #endif
            // Buffer is loaded only if this is not the last message.
            if (i_packet+1 != packet_buffer_n_packets_) {
                packet_buffer_loaded_ = true;
                // Continue on next packet.
                packet_buffer_offset_ = i_packet + 1;

            // If i_packet is the last packet the buffer is empty.
            } else {
                packet_buffer_loaded_ = false;
                packet_buffer_offset_ = 0;
            }

            return metadata.pulse_id;
        }
    }
    // We emptied the buffer.
    packet_buffer_loaded_ = false;
    packet_buffer_offset_ = 0;

    return 0;
}

uint64_t FrameUdpReceiver::get_frame_from_udp(
        ModuleFrame& metadata, char* frame_buffer)
{
    // Reset the metadata and frame buffer for the next frame.
    metadata.pulse_id = 0;
    metadata.n_recv_packets = 0;
    memset(frame_buffer, 0, data_bytes_per_frame_);	
    
    // Happens when last packet from previous frame was missed.
    if (packet_buffer_loaded_) {

        auto pulse_id = process_packets(
                packet_buffer_offset_, metadata, frame_buffer);
        if (pulse_id != 0) {
            return pulse_id;
        }
    }

    while (true) {

        packet_buffer_n_packets_ = udp_receiver_.receive_many(
                msgs_, BUFFER_UDP_N_RECV_MSG);

        if (packet_buffer_n_packets_ == 0) {
            continue;
        }

        auto pulse_id = process_packets(0, metadata, frame_buffer);

        if (pulse_id != 0) {
            return pulse_id;
        }
    }
}
