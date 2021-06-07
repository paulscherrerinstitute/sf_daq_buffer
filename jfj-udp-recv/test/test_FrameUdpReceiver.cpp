#include <netinet/in.h>
#include <jungfraujoch.hpp>
#include "gtest/gtest.h"
#include "JfjFrameUdpReceiver.hpp"
#include "mock/udp.hpp"

#include <thread>
#include <chrono>
#include <future>

using namespace std;


void mockDetector(int& udp_socket_fd, int64_t n_frames){
    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);

    for (int64_t i_frame=0; i_frame < n_frames; i_frame++){
        for (size_t i_packet=0; i_packet<JFJOCH_N_PACKETS_PER_FRAME; i_packet++) {
            jfjoch_packet_t send_udp_buffer;
            send_udp_buffer.packetnum = i_packet;
            send_udp_buffer.bunchid = i_frame + 1;
            send_udp_buffer.framenum = i_frame + 1000;
            send_udp_buffer.debug = i_frame + 10000;

            ::sendto(udp_socket_fd, &send_udp_buffer, sizeof(send_udp_buffer),
                    0, (sockaddr*) &server_address, sizeof(server_address));

        }
    }
    std::cout << "Sent " << n_frames << " frames\t" << JFJOCH_N_PACKETS_PER_FRAME*n_frames << " packets" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));    
}
    

TEST(BufferUdpReceiver, simple_recv){
    // Open detector port
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);    
    
    
    int n_frames = 3;


    // Open receiver
    JfjFrameUdpReceiver udp_receiver(MOCK_UDP_PORT);

    std::cout << "NI" << std::endl;
    auto handle = async(launch::async, [&]{ mockDetector(send_socket_fd, n_frames); } );
    std::cout << "NI before wait()" << std::endl;

    handle.wait();
    std::cout << "NI after wait()" << std::endl;

    ModuleFrame metadata;
    auto frame_buffer = make_unique<char[]>(JFJOCH_DATA_BYTES_PER_FRAME);
    std::cout << "NI" << std::endl;

    for (int i_frame=0; i_frame < n_frames; i_frame++) {
        std::cout << "Getting frame: " << i_frame <<  std::endl;
        auto pulse_id = udp_receiver.get_frame_from_udp(metadata, frame_buffer.get());
        std::cout << ".. gotcha " << pulse_id <<  std::endl;

        ASSERT_EQ(i_frame + 1, pulse_id);
        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(metadata.n_recv_packets, JFJOCH_N_PACKETS_PER_FRAME);
    }
    std::cout << "NI" << std::endl;

    ::close(send_socket_fd);
}

TEST(BufferUdpReceiver, missing_middle_packet)
{
    int n_packets = JFJOCH_N_PACKETS_PER_FRAME;
    int n_frames = 3;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    JfjFrameUdpReceiver udp_receiver(udp_port);

    auto handle = async(launch::async, [&](){
        for (int64_t i_frame=0; i_frame < n_frames; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
                // Skip some random middle packet.
                if (i_packet == 10) {
                    continue;
                }

                jfjoch_packet_t send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame + 1;
                send_udp_buffer.framenum = i_frame + 1000;
                send_udp_buffer.debug = i_frame + 10000;

                ::sendto(
                        send_socket_fd,
                        &send_udp_buffer,
                        JFJOCH_BYTES_PER_PACKET,
                        0,
                        (sockaddr*) &server_address,
                        sizeof(server_address));
            }
        }
    });

    handle.wait();

    ModuleFrame metadata;
    auto frame_buffer = make_unique<char[]>(JFJOCH_DATA_BYTES_PER_FRAME);

    for (int i_frame=0; i_frame < n_frames; i_frame++) {
        auto pulse_id = udp_receiver.get_frame_from_udp(
                metadata, frame_buffer.get());

        ASSERT_EQ(i_frame + 1, pulse_id);
        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(metadata.n_recv_packets, n_packets - 1);
    }

    ::close(send_socket_fd);
}

TEST(BufferUdpReceiver, missing_first_packet)
{
    auto n_packets = JFJOCH_N_PACKETS_PER_FRAME;
    int n_frames = 3;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    JfjFrameUdpReceiver udp_receiver(udp_port);

    auto handle = async(launch::async, [&](){
        for (int64_t i_frame=0; i_frame < n_frames; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
                // Skip first packet.
                if (i_packet == 0) {
                    continue;
                }

                jfjoch_packet_t send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame + 1;
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
    auto frame_buffer = make_unique<char[]>(JFJOCH_DATA_BYTES_PER_FRAME);

    for (int i_frame=0; i_frame < n_frames; i_frame++) {
        auto pulse_id = udp_receiver.get_frame_from_udp(
                metadata, frame_buffer.get());

        ASSERT_EQ(i_frame + 1, pulse_id);
        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(metadata.n_recv_packets, n_packets - 1);
    }

    ::close(send_socket_fd);
}

TEST(BufferUdpReceiver, missing_last_packet)
{
    int n_packets = JFJOCH_N_PACKETS_PER_FRAME;
    int n_frames = 3;

    uint16_t udp_port = MOCK_UDP_PORT;
    auto server_address = get_server_address(udp_port);
    auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_TRUE(send_socket_fd >= 0);

    JfjFrameUdpReceiver udp_receiver(udp_port);

    auto handle = async(launch::async, [&](){
        for (int64_t i_frame=0; i_frame < n_frames; i_frame++){
            for (size_t i_packet=0; i_packet<n_packets; i_packet++) {
                // Skip the last packet.
                if (i_packet == n_packets-1) {
                    continue;
                }

                jfjoch_packet_t send_udp_buffer;
                send_udp_buffer.packetnum = i_packet;
                send_udp_buffer.bunchid = i_frame + 1;
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
    auto frame_buffer = make_unique<char[]>(JFJOCH_DATA_BYTES_PER_FRAME);

    // n_frames -1 because the last frame is not complete.
    for (int i_frame=0; i_frame < n_frames - 1; i_frame++) {
        auto pulse_id = udp_receiver.get_frame_from_udp(metadata, frame_buffer.get());

        ASSERT_EQ(i_frame + 1, pulse_id);
        ASSERT_EQ(metadata.frame_index, i_frame + 1000);
        ASSERT_EQ(metadata.daq_rec, i_frame + 10000);
        // -1 because we skipped a packet.
        ASSERT_EQ(metadata.n_recv_packets, n_packets - 1);
    }

    ::close(send_socket_fd);
}
