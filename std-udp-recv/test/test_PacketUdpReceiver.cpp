#include <netinet/in.h>
#include "gtest/gtest.h"
#include "mock/udp.hpp"
#include "PacketUdpReceiver.hpp"

#include <thread>
#include <chrono>

#ifdef USE_EIGER
#include "eiger.hpp"
#else
#include "jungfrau.hpp"
#endif

using namespace std;

const int N_PACKETS_PER_FRAME = 128;

TEST(PacketUdpReceiver, simple_recv)
{
    uint16_t udp_port = MOCK_UDP_PORT;

    auto send_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    ASSERT_TRUE(send_socket_fd >= 0);

    PacketUdpReceiver udp_receiver;
    udp_receiver.bind(udp_port);

    det_packet send_udp_buffer;
    send_udp_buffer.packetnum = 91;
    send_udp_buffer.framenum = 92;
    send_udp_buffer.bunchid = 93;
    send_udp_buffer.debug = 94;

    auto server_address = get_server_address(udp_port);
    ::sendto(
            send_socket_fd,
            &send_udp_buffer,
            BYTES_PER_PACKET,
            0,
            (sockaddr*) &server_address,
            sizeof(server_address));

    this_thread::sleep_for(chrono::milliseconds(100));

    det_packet recv_udp_buffer;
    ASSERT_TRUE(udp_receiver.receive(
            &recv_udp_buffer, BYTES_PER_PACKET));

    EXPECT_EQ(send_udp_buffer.packetnum, recv_udp_buffer.packetnum);
    EXPECT_EQ(send_udp_buffer.framenum, recv_udp_buffer.framenum);
    EXPECT_EQ(send_udp_buffer.bunchid, recv_udp_buffer.bunchid);
    EXPECT_EQ(send_udp_buffer.debug, recv_udp_buffer.debug);

    ASSERT_FALSE(udp_receiver.receive(
            &recv_udp_buffer, BYTES_PER_PACKET));

    udp_receiver.disconnect();
    ::close(send_socket_fd);
}

TEST(PacketUdpReceiver, false_recv)
{
    uint16_t udp_port = MOCK_UDP_PORT;

    auto send_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    ASSERT_TRUE(send_socket_fd >= 0);

    PacketUdpReceiver udp_receiver;
    udp_receiver.bind(udp_port);

    det_packet send_udp_buffer;
    det_packet recv_udp_buffer;

    auto server_address = get_server_address(udp_port);

    ::sendto(
            send_socket_fd,
            &send_udp_buffer,
            BYTES_PER_PACKET-1,
            0,
            (sockaddr*) &server_address,
            sizeof(server_address));

    ASSERT_FALSE(udp_receiver.receive(
            &recv_udp_buffer, BYTES_PER_PACKET));

    ::sendto(
            send_socket_fd,
            &send_udp_buffer,
            BYTES_PER_PACKET,
            0,
            (sockaddr*) &server_address,
            sizeof(server_address));

    ASSERT_TRUE(udp_receiver.receive(
            &recv_udp_buffer, BYTES_PER_PACKET));

    ::sendto(
            send_socket_fd,
            &send_udp_buffer,
            BYTES_PER_PACKET-1,
            0,
            (sockaddr*) &server_address,
            sizeof(server_address));

    ASSERT_TRUE(udp_receiver.receive(
            &recv_udp_buffer, BYTES_PER_PACKET-1));

    udp_receiver.disconnect();
    ::close(send_socket_fd);
}

TEST(PacketUdpReceiver, receive_many)
{
    auto n_msg_buffer = N_PACKETS_PER_FRAME;
    det_packet recv_buffer[n_msg_buffer];
    iovec recv_buff_ptr[n_msg_buffer];
    struct mmsghdr msgs[n_msg_buffer];
    struct sockaddr_in sockFrom[n_msg_buffer];

    for (int i = 0; i < n_msg_buffer; i++) {
        recv_buff_ptr[i].iov_base = (void*) &(recv_buffer[i]);
        recv_buff_ptr[i].iov_len = sizeof(det_packet);

        msgs[i].msg_hdr.msg_iov = &recv_buff_ptr[i];
        msgs[i].msg_hdr.msg_iovlen = 1;
        msgs[i].msg_hdr.msg_name = &sockFrom[i];
        msgs[i].msg_hdr.msg_namelen = sizeof(sockaddr_in);
    }

    uint16_t udp_port = MOCK_UDP_PORT;

    auto send_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    ASSERT_TRUE(send_socket_fd >= 0);

    PacketUdpReceiver udp_receiver;
    udp_receiver.bind(udp_port);

    det_packet send_udp_buffer;

    auto server_address = get_server_address(udp_port);

    send_udp_buffer.bunchid = 0;
    ::sendto(
            send_socket_fd,
            &send_udp_buffer,
            BYTES_PER_PACKET,
            0,
            (sockaddr*) &server_address,
            sizeof(server_address));

    send_udp_buffer.bunchid = 1;
    ::sendto(
            send_socket_fd,
            &send_udp_buffer,
            BYTES_PER_PACKET,
            0,
            (sockaddr*) &server_address,
            sizeof(server_address));

    this_thread::sleep_for(chrono::milliseconds(10));

    auto n_msgs = udp_receiver.receive_many(msgs, N_PACKETS_PER_FRAME);
    ASSERT_EQ(n_msgs, 2);

    for (size_t i=0;i<n_msgs;i++) {
        ASSERT_EQ(msgs[i].msg_len, BYTES_PER_PACKET);
        ASSERT_EQ(recv_buffer[i].bunchid, i);
    }

    n_msgs = udp_receiver.receive_many(msgs, N_PACKETS_PER_FRAME);
    ASSERT_EQ(n_msgs, -1);

    udp_receiver.disconnect();
    ::close(send_socket_fd);
}