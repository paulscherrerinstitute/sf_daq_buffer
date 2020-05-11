#include <netinet/in.h>
#include <jungfrau.hpp>
#include "gtest/gtest.h"
#include "BufferUdpReceiver.hpp"
#include "mock/udp.hpp"

#include <thread>
#include <chrono>
#include <future>

using namespace std;

TEST(BufferUdpReceiver, simple_recv)
{
    auto n_packets = JUNGFRAU_N_PACKETS_PER_FRAME;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    BufferUdpReceiver udp_receiver(udp_port, 1);
    this_thread::sleep_for(chrono::milliseconds(100));

    uint64_t send_pulse_id = 1234567;

    auto handle = async(launch::async, [&](){
        for (size_t i_packet=0; i_packet<n_packets; i_packet++) {

            jungfrau_packet send_udp_buffer;
            send_udp_buffer.packetnum = i_packet;
            send_udp_buffer.bunchid = send_pulse_id;
            send_udp_buffer.framenum = i_packet + 1000;
            send_udp_buffer.debug = i_packet + 100000;

            ::sendto(
                    send_socket_fd,
                    &send_udp_buffer,
                    JUNGFRAU_BYTES_PER_PACKET,
                    0,
                    (sockaddr*) &server_address,
                    sizeof(server_address));
        }
    });

    handle.wait();

    ModuleFrame metadata;
    auto frame_buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);

    auto pulse_id = udp_receiver.get_frame_from_udp(
            metadata, frame_buffer.get());
    ASSERT_EQ(send_pulse_id, pulse_id);

    ::close(send_socket_fd);
}
