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
    int source_id = 1234;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    BufferUdpReceiver udp_receiver(udp_port, source_id);

    auto handle = async(launch::async, [&](){
        for (int i_frame=0; i_frame < BUFFER_UDP_RCVBUF_N_SLOTS; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {

                jungfrau_packet send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame;
                send_udp_buffer.framenum = i_frame + 1000;
                send_udp_buffer.debug = i_frame + 10000;

                ::sendto(
                        send_socket_fd,
                        &send_udp_buffer,
                        JUNGFRAU_BYTES_PER_PACKET,
                        0,
                        (sockaddr*) &server_address,
                        sizeof(server_address));
            }
        }
    });

    handle.wait();

    ModuleFrame metadata;
    auto frame_buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);

    for (int i_frame=0; i_frame < BUFFER_UDP_RCVBUF_N_SLOTS; i_frame++) {
        auto pulse_id = udp_receiver.get_frame_from_udp(
                metadata, frame_buffer.get());
        ASSERT_EQ(i_frame, pulse_id);
        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
        ASSERT_EQ(metadata.n_received_packets, n_packets);
        ASSERT_EQ(metadata.module_id, source_id);
    }

    ::close(send_socket_fd);
}
