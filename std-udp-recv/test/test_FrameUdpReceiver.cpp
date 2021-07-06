#include <netinet/in.h>
#include "gtest/gtest.h"
#include "FrameUdpReceiver.hpp"
#include "mock/udp.hpp"

#include <thread>
#include <chrono>
#include <future>

using namespace std;

const int DATA_BYTES_PER_FRAME = 512*1024*2;

TEST(BufferUdpReceiver, simple_recv)
{
    auto n_packets = 128;
    int module_id = 0;
    int n_frames = 5;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    FrameUdpReceiver udp_receiver(udp_port, n_packets);

    auto handle = async(launch::async, [&](){
        for (int i_frame=0; i_frame < n_frames; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
                det_packet send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame + 1;
                send_udp_buffer.framenum = i_frame + 1000;
                send_udp_buffer.debug = i_frame + 10000;

                ::sendto(
                        send_socket_fd,
                        &send_udp_buffer,
                        BYTES_PER_PACKET,
                        0,
                        (sockaddr*) &server_address,
                        sizeof(server_address));
            }
        }
    });

    handle.wait();

    ModuleFrame meta;
    meta.module_id = module_id;
    meta.bit_depth = 16;
    auto frame_buffer = make_unique<char[]>(DATA_BYTES_PER_FRAME);

    for (int i_frame=0; i_frame < n_frames; i_frame++) {
        udp_receiver.get_frame_from_udp(meta, frame_buffer.get());

        ASSERT_EQ(i_frame + 1, meta.pulse_id);
        ASSERT_EQ(meta.frame_index, i_frame + 1000);
        ASSERT_EQ(meta.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(meta.n_recv_packets, n_packets);
        ASSERT_EQ(meta.module_id, module_id);
    }

    ::close(send_socket_fd);
}

TEST(BufferUdpReceiver, missing_middle_packet)
{
    auto n_packets = 128;
    int module_id = 1234;
    int n_frames = 3;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    FrameUdpReceiver udp_receiver(udp_port, n_packets);

    auto handle = async(launch::async, [&](){
        for (int i_frame=0; i_frame < n_frames; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
                // Skip some random middle packet.
                if (i_packet == 10) {
                    continue;
                }

                det_packet send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame + 1;
                send_udp_buffer.framenum = i_frame + 1000;
                send_udp_buffer.debug = i_frame + 10000;

                ::sendto(
                        send_socket_fd,
                        &send_udp_buffer,
                        BYTES_PER_PACKET,
                        0,
                        (sockaddr*) &server_address,
                        sizeof(server_address));
            }
        }
    });

    handle.wait();

    ModuleFrame meta;
    meta.module_id = module_id;

    auto frame_buffer = make_unique<char[]>(DATA_BYTES_PER_FRAME);

    for (int i_frame=0; i_frame < n_frames; i_frame++) {
        udp_receiver.get_frame_from_udp(meta, frame_buffer.get());

        ASSERT_EQ(i_frame + 1, meta.pulse_id);
        ASSERT_EQ(meta.frame_index, i_frame + 1000);
        ASSERT_EQ(meta.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(meta.n_recv_packets, n_packets - 1);
        ASSERT_EQ(meta.module_id, module_id);
    }

    ::close(send_socket_fd);
}

TEST(BufferUdpReceiver, missing_first_packet)
{
    auto n_packets = 128;
    int module_id = 1234;
    int n_frames = 3;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    FrameUdpReceiver udp_receiver(udp_port, n_packets);

    auto handle = async(launch::async, [&](){
        for (int i_frame=0; i_frame < n_frames; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
                // Skip first packet.
                if (i_packet == 0) {
                    continue;
                }

                det_packet send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame + 1;
                send_udp_buffer.framenum = i_frame + 1000;
                send_udp_buffer.debug = i_frame + 10000;

                ::sendto(
                        send_socket_fd,
                        &send_udp_buffer,
                        BYTES_PER_PACKET,
                        0,
                        (sockaddr*) &server_address,
                        sizeof(server_address));
            }
        }
    });

    handle.wait();

    ModuleFrame meta;
    meta.module_id = module_id;
    auto frame_buffer = make_unique<char[]>(DATA_BYTES_PER_FRAME);

    for (int i_frame=0; i_frame < n_frames; i_frame++) {
        udp_receiver.get_frame_from_udp(meta, frame_buffer.get());

        ASSERT_EQ(i_frame + 1, meta.pulse_id);
        ASSERT_EQ(meta.frame_index, i_frame + 1000);
        ASSERT_EQ(meta.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(meta.n_recv_packets, n_packets - 1);
        ASSERT_EQ(meta.module_id, module_id);
    }

    ::close(send_socket_fd);
}

TEST(BufferUdpReceiver, missing_last_packet)
{
    auto n_packets = 128;
    int module_id = 1234;
    int n_frames = 3;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    FrameUdpReceiver udp_receiver(udp_port, n_packets);

    auto handle = async(launch::async, [&](){
        for (int i_frame=0; i_frame < n_frames; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
                // Skip the last packet.
                if (i_packet == n_packets-1) {
                    continue;
                }

                det_packet send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame + 1;
                send_udp_buffer.framenum = i_frame + 1000;
                send_udp_buffer.debug = i_frame + 10000;

                ::sendto(
                        send_socket_fd,
                        &send_udp_buffer,
                        BYTES_PER_PACKET,
                        0,
                        (sockaddr*) &server_address,
                        sizeof(server_address));
            }
        }
    });

    handle.wait();

    ModuleFrame meta;
    meta.module_id = module_id;
    auto frame_buffer = make_unique<char[]>(DATA_BYTES_PER_FRAME);

    // n_frames -1 because the last frame is not complete.
    for (int i_frame=0; i_frame < n_frames - 1; i_frame++) {
        udp_receiver.get_frame_from_udp(meta, frame_buffer.get());

        ASSERT_EQ(i_frame + 1, meta.pulse_id);
        ASSERT_EQ(meta.frame_index, i_frame + 1000);
        ASSERT_EQ(meta.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(meta.n_recv_packets, n_packets - 1);
        ASSERT_EQ(meta.module_id, module_id);
    }

    ::close(send_socket_fd);
}