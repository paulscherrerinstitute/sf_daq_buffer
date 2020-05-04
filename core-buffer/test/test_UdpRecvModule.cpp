#include "gtest/gtest.h"
#include "UdpRecvModule.hpp"
#include "jungfrau.hpp"
#include "mock/udp.hpp"

using namespace std;

TEST(UdpRecvModule, basic_interaction)
{
    uint16_t udp_port(MOCK_UDP_PORT);

    FastQueue<ModuleFrame> queue(JUNGFRAU_DATA_BYTES_PER_FRAME, 10);
    UdpRecvModule udp_recv_module(queue, udp_port);
}

TEST(UdpRecvModule, simple_recv)
{
    int slot_id;
    uint16_t udp_port(MOCK_UDP_PORT);
    size_t n_msg(128);

    FastQueue<ModuleFrame> queue(JUNGFRAU_DATA_BYTES_PER_FRAME, 10);
    UdpRecvModule udp_recv_module(queue, udp_port);

    this_thread::sleep_for(chrono::milliseconds(100));

    // The first slot should not be available to read yet.
    ASSERT_EQ(queue.read(), -1);

    auto send_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    ASSERT_TRUE(send_socket_fd >= 0);

    auto server_address = get_server_address(udp_port);

    jungfrau_packet send_udp_buffer;
    send_udp_buffer.bunchid = 100;
    send_udp_buffer.debug = 1000;

    send_udp_buffer.framenum = 1;
    for (size_t i=0; i<n_msg; i++) {
        send_udp_buffer.packetnum = i;

        ::sendto(
                send_socket_fd,
                &send_udp_buffer,
                JUNGFRAU_BYTES_PER_PACKET,
                0,
                (sockaddr*) &server_address,
                sizeof(server_address));

    }

    this_thread::sleep_for(chrono::milliseconds(100));

    slot_id = queue.read();
    // This time we are supposed to get slot 0.
    ASSERT_EQ(slot_id, 0);
    // We sent a frame with frame_index == 1.
    ASSERT_EQ(queue.get_metadata_buffer(slot_id)->frame_index, 1);
    queue.release();

    // Next slot not yet ready.
    ASSERT_EQ(queue.read(), -1);

    send_udp_buffer.framenum = 2;
    for (size_t i=0; i<128; i++){
        send_udp_buffer.packetnum = i;

        ::sendto(
                send_socket_fd,
                &send_udp_buffer,
                JUNGFRAU_BYTES_PER_PACKET,
                0,
                (sockaddr*) &server_address,
                sizeof(server_address));

    }

    this_thread::sleep_for(chrono::milliseconds(100));

    slot_id = queue.read();
    // This time we are supposed to get slot 1.
    ASSERT_EQ(slot_id, 1);
    // We sent a frame with frame_index == 2.
    ASSERT_EQ(queue.get_metadata_buffer(slot_id)->frame_index, 2);
    queue.release();

    ::close(send_socket_fd);
}
