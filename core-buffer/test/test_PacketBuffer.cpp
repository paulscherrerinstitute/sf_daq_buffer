#include <netinet/in.h>
#include <jungfraujoch.hpp>
#include "gtest/gtest.h"
#include "PacketBuffer.hpp"

#include <thread>
#include <chrono>
#include <future>

using namespace std;

template <typename TY>
class MockReceiver<TY>{
    public:
        int idx_packet = 42000;
        int packet_per_frame = 512;
        int num_bunches = 100;
        int num_packets =50;

        int receive_many(mmsghdr* msgs, const size_t n_msgs){
            // Receive 'num_packets numner of packets'

            for(int ii=0; ii<std::min(num_packets, n_msgs); ii++){
                jfjoch_packet_t& refer = std::reinterpret_cast<jfjoch_packet_t&>(mmsghdr[ii].msg_hdr.msg_iov->iov_base);
                refer.bunchid = idx_packet / packet_per_frame;
                refer.packetnum = idx_packet % packet_per_frame;
                idx_packet++;
            }
            return std::min(num_packets, n_msgs);
        };
};

//
//
//
//
//TEST(BufferUdpReceiver, simple_recv)
//{
//    auto n_packets = JF_N_PACKETS_PER_FRAME;
//    int n_frames = 5;
//
//    uint16_t udp_port = MOCK_UDP_PORT;
//    auto server_address = get_server_address(udp_port);
//    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
//    ASSERT_TRUE(send_socket_fd >= 0);
//
//    JfjFrameUdpReceiver udp_receiver(udp_port);
//
//    auto handle = async(launch::async, [&](){
//        for (int i_frame=0; i_frame < n_frames; i_frame++){
//            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
//                jungfrau_packet send_udp_buffer;
//                send_udp_buffer.packetnum = i_packet;
//                send_udp_buffer.bunchid = i_frame + 1;
//                send_udp_buffer.framenum = i_frame + 1000;
//                send_udp_buffer.debug = i_frame + 10000;
//
//                ::sendto(
//                        send_socket_fd,
//                        &send_udp_buffer,
//                        JUNGFRAU_BYTES_PER_PACKET,
//                        0,
//                        (sockaddr*) &server_address,
//                        sizeof(server_address));
//            }
//        }
//    });
//
//    handle.wait();
//
//    ModuleFrame metadata;
//    auto frame_buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);
//
//    for (int i_frame=0; i_frame < n_frames; i_frame++) {
//        auto pulse_id = udp_receiver.get_frame_from_udp(
//                metadata, frame_buffer.get());
//
//        ASSERT_EQ(i_frame + 1, pulse_id);
//        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
//        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
//        // -1 because we skipped a packet.
//        ASSERT_EQ(metadata.n_recv_packets, n_packets);
//    }
//
//    ::close(send_socket_fd);
//}
//
//TEST(BufferUdpReceiver, missing_middle_packet)
//{
//    auto n_packets = JF_N_PACKETS_PER_FRAME;
//    int n_frames = 3;
//
//    uint16_t udp_port = MOCK_UDP_PORT;
//    auto server_address = get_server_address(udp_port);
//    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
//    ASSERT_TRUE(send_socket_fd >= 0);
//
//    JfjFrameUdpReceiver udp_receiver(udp_port, source_id);
//
//    auto handle = async(launch::async, [&](){
//        for (int i_frame=0; i_frame < n_frames; i_frame++){
//            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
//                // Skip some random middle packet.
//                if (i_packet == 10) {
//                    continue;
//                }
//
//                jungfrau_packet send_udp_buffer;
//                send_udp_buffer.packetnum = i_packet;
//                send_udp_buffer.bunchid = i_frame + 1;
//                send_udp_buffer.framenum = i_frame + 1000;
//                send_udp_buffer.debug = i_frame + 10000;
//
//                ::sendto(
//                        send_socket_fd,
//                        &send_udp_buffer,
//                        JUNGFRAU_BYTES_PER_PACKET,
//                        0,
//                        (sockaddr*) &server_address,
//                        sizeof(server_address));
//            }
//        }
//    });
//
//    handle.wait();
//
//    ModuleFrame metadata;
//    auto frame_buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);
//
//    for (int i_frame=0; i_frame < n_frames; i_frame++) {
//        auto pulse_id = udp_receiver.get_frame_from_udp(
//                metadata, frame_buffer.get());
//
//        ASSERT_EQ(i_frame + 1, pulse_id);
//        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
//        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
//        // -1 because we skipped a packet.
//        ASSERT_EQ(metadata.n_recv_packets, n_packets - 1);
//    }
//
//    ::close(send_socket_fd);
//}
//
//TEST(BufferUdpReceiver, missing_first_packet)
//{
//    auto n_packets = JF_N_PACKETS_PER_FRAME;
//    int n_frames = 3;
//
//    uint16_t udp_port = MOCK_UDP_PORT;
//    auto server_address = get_server_address(udp_port);
//    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
//    ASSERT_TRUE(send_socket_fd >= 0);
//
//    JfjFrameUdpReceiver udp_receiver(udp_port);
//
//    auto handle = async(launch::async, [&](){
//        for (int i_frame=0; i_frame < n_frames; i_frame++){
//            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
//                // Skip first packet.
//                if (i_packet == 0) {
//                    continue;
//                }
//
//                jungfrau_packet send_udp_buffer;
//                send_udp_buffer.packetnum = i_packet;
//                send_udp_buffer.bunchid = i_frame + 1;
//                send_udp_buffer.framenum = i_frame + 1000;
//                send_udp_buffer.debug = i_frame + 10000;
//
//                ::sendto(
//                        send_socket_fd,
//                        &send_udp_buffer,
//                        JUNGFRAU_BYTES_PER_PACKET,
//                        0,
//                        (sockaddr*) &server_address,
//                        sizeof(server_address));
//            }
//        }
//    });
//
//    handle.wait();
//
//    ModuleFrame metadata;
//    auto frame_buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);
//
//    for (int i_frame=0; i_frame < n_frames; i_frame++) {
//        auto pulse_id = udp_receiver.get_frame_from_udp(
//                metadata, frame_buffer.get());
//
//        ASSERT_EQ(i_frame + 1, pulse_id);
//        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
//        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
//        // -1 because we skipped a packet.
//        ASSERT_EQ(metadata.n_recv_packets, n_packets - 1);
//    }
//
//    ::close(send_socket_fd);
//}
//
//TEST(BufferUdpReceiver, missing_last_packet)
//{
//    auto n_packets = JF_N_PACKETS_PER_FRAME;
//    int n_frames = 3;
//
//    uint16_t udp_port = MOCK_UDP_PORT;
//    auto server_address = get_server_address(udp_port);
//    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
//    ASSERT_TRUE(send_socket_fd >= 0);
//
//    JfjFrameUdpReceiver udp_receiver(udp_port);
//
//    auto handle = async(launch::async, [&](){
//        for (int i_frame=0; i_frame < n_frames; i_frame++){
//            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
//                // Skip the last packet.
//                if (i_packet == n_packets-1) {
//                    continue;
//                }
//
//                jungfrau_packet send_udp_buffer;
//                send_udp_buffer.packetnum = i_packet;
//                send_udp_buffer.bunchid = i_frame + 1;
//                send_udp_buffer.framenum = i_frame + 1000;
//                send_udp_buffer.debug = i_frame + 10000;
//
//                ::sendto(
//                        send_socket_fd,
//                        &send_udp_buffer,
//                        JUNGFRAU_BYTES_PER_PACKET,
//                        0,
//                        (sockaddr*) &server_address,
//                        sizeof(server_address));
//            }
//        }
//    });
//
//    handle.wait();
//
//    ModuleFrame metadata;
//    auto frame_buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);
//
//    // n_frames -1 because the last frame is not complete.
//    for (int i_frame=0; i_frame < n_frames - 1; i_frame++) {
//        auto pulse_id = udp_receiver.get_frame_from_udp(metadata, frame_buffer.get());
//
//        ASSERT_EQ(i_frame + 1, pulse_id);
//        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
//        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
//        // -1 because we skipped a packet.
//        ASSERT_EQ(metadata.n_recv_packets, n_packets - 1);
//    }
//
//    ::close(send_socket_fd);
//}
