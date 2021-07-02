#include <netinet/in.h>
#include <iostream>
#include <stdexcept>
#include "PacketUdpReceiver.hpp"
#include "jungfrau.hpp"
#include <unistd.h>
#include <cstring>
#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

PacketUdpReceiver::PacketUdpReceiver() :
    socket_fd_(-1)
{
}

PacketUdpReceiver::~PacketUdpReceiver()
{
    disconnect();
}

void PacketUdpReceiver::bind(const uint16_t port)
{
    if (socket_fd_ > -1) {
        throw runtime_error("Socket already bound.");
    }

    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        throw runtime_error("Cannot open socket.");
    }

    sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    timeval udp_socket_timeout;
    udp_socket_timeout.tv_sec = 0;
    udp_socket_timeout.tv_usec = BUFFER_UDP_US_TIMEOUT;

    if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO,
            &udp_socket_timeout, sizeof(timeval)) == -1) {
        throw runtime_error(
                "Cannot set SO_RCVTIMEO. " + string(strerror(errno)));
    }

    if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF,
                   &BUFFER_UDP_RCVBUF_BYTES, sizeof(int)) == -1) {
        throw runtime_error(
                "Cannot set SO_RCVBUF. " + string(strerror(errno)));
    };
    //TODO: try to set SO_RCVLOWAT

    auto bind_result = ::bind(
            socket_fd_,
            reinterpret_cast<const sockaddr *>(&server_address),
            sizeof(server_address));

    if (bind_result < 0) {
        throw runtime_error("Cannot bind socket.");
    }
}

int PacketUdpReceiver::receive_many(mmsghdr* msgs, const size_t n_msgs)
{
    return recvmmsg(socket_fd_, msgs, n_msgs, 0, 0);
}

bool PacketUdpReceiver::receive(void* buffer, const size_t buffer_n_bytes)
{
    auto data_len = recv(socket_fd_, buffer, buffer_n_bytes, 0);

    if (data_len < 0) {
        return false;
    }

    if (data_len != buffer_n_bytes) {
        return false;
    }

    return true;
}

void PacketUdpReceiver::disconnect()
{
    close(socket_fd_);
    socket_fd_ = -1;
}
