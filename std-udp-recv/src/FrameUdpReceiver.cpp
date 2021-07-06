#include <cstring>
#include "FrameUdpReceiver.hpp"
#include <ostream>
#include <iostream>
#include <chrono>
#include "date.h"

using namespace std;
using namespace buffer_config;

FrameUdpReceiver::FrameUdpReceiver(
        const uint16_t port, const int n_packets_per_frame) :
            n_packets_per_frame_(n_packets_per_frame),
            packet_buffer_(new det_packet[n_packets_per_frame_]),
            recv_buff_ptr_(new iovec[n_packets_per_frame_]),
            msgs_(new mmsghdr[n_packets_per_frame_]),
            sock_from_(new sockaddr_in[n_packets_per_frame_])
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [FrameUdpReceiver::FrameUdpReceiver] :";
        cout << " Details of FrameUdpReceiver:";
        cout << " || port: " << port;
        cout << " || n_packets_per_frame: " << n_packets_per_frame_;
        cout << endl;
    #endif

    udp_receiver_.bind(port);

    for (int i = 0; i < n_packets_per_frame_; i++) {
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

    delete[] packet_buffer_;
    delete[] recv_buff_ptr_;
    delete[] msgs_;
    delete[] sock_from_;
}

inline void FrameUdpReceiver::init_frame(
        ModuleFrame& frame_metadata, const int i_packet)
{
    frame_metadata.pulse_id = packet_buffer_[i_packet].bunchid;
    frame_metadata.frame_index = packet_buffer_[i_packet].framenum;
    frame_metadata.daq_rec = (uint64_t) packet_buffer_[i_packet].debug;
    frame_metadata.pos_y = (int16_t) packet_buffer_[i_packet].row;
    frame_metadata.pos_x = (int16_t) packet_buffer_[i_packet].column;
    frame_metadata.n_recv_packets = 0;

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [FrameUdpReceiver::init_frame] :";
        cout << " || pos_y: " << frame_metadata.pos_y;
        cout << " || pos_x: " << frame_metadata.pos_x;
        cout << " || pulse_id: " << frame_metadata.pulse_id;
        cout << " || frame_index: " << frame_metadata.frame_index;
        cout << " || daq_rec: " << frame_metadata.daq_rec;
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
    if (i_packet == 0){
        // cout << " BUFFER LOCATION " << (frame_buffer + frame_buffer_offset) << endl;
    }
    metadata.n_recv_packets++;
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
        if (metadata.frame_index == INVALID_FRAME_INDEX) {
            init_frame(metadata, i_packet);

        // Happens if the last packet from the previous frame gets lost.
        // In the jungfrau_packet, framenum is the trigger number
        // (how many triggers from detector power-on) happened
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
                cout << " frame " << metadata.frame_index << " || ";
                cout << packet_buffer_[i_packet].packetnum+1;
                cout << " packets received.";
                cout << endl;
            #endif
            // buffer is loaded only if this is not the last message.
            if (i_packet+1 != packet_buffer_n_packets_) {
                packet_buffer_loaded_ = true;
                // continue on next packet.
                packet_buffer_offset_ = i_packet + 1;

            // if i_packet is the last packet the buffer is empty.
            } else {
                packet_buffer_loaded_ = false;
                packet_buffer_offset_ = 0;
            }

            return metadata.frame_index;
        }
    }
    // We emptied the buffer.
    packet_buffer_loaded_ = false;
    packet_buffer_offset_ = 0;

    return INVALID_FRAME_INDEX;
}

uint64_t FrameUdpReceiver::get_frame_from_udp(
        ModuleFrame& meta, char* frame_buffer)
{
    meta.frame_index = INVALID_FRAME_INDEX;

    // Happens when last packet from previous frame was missed.
    if (packet_buffer_loaded_) {

        auto frame_index = process_packets(
                packet_buffer_offset_, meta, frame_buffer);
        if (frame_index != INVALID_FRAME_INDEX) {
            return frame_index;
        }
    }

    while (true) {

        packet_buffer_n_packets_ = udp_receiver_.receive_many(
                msgs_, n_packets_per_frame_);

        if (packet_buffer_n_packets_ == 0) {
            continue;
        }

        auto frame_index = process_packets(0, meta, frame_buffer);

        if (frame_index != INVALID_FRAME_INDEX) {
            return frame_index;
        }
    }
}
