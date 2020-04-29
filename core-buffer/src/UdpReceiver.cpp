#include <netinet/in.h>
#include <iostream>
#include "UdpReceiver.hpp"
#include "jungfrau.hpp"
#include "date.h"
#include <unistd.h>

using namespace std;

UdpReceiver::UdpReceiver() :
    socket_fd_(-1)
{
}

UdpReceiver::~UdpReceiver()
{
    disconnect();
}

void UdpReceiver::bind(const uint16_t port, const size_t usec_timeout)
{
    if (socket_fd_ > -1) {
        throw runtime_error("Socket already bound.");
    }

    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        throw runtime_error("Cannot open socket.");
    }

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[UdpReceiver::bind]";
        cout << " port " << port;
        cout << " usec_timeout " << usec_timeout << endl;
    #endif

    sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    auto bind_result = ::bind(
            socket_fd_,
            reinterpret_cast<const sockaddr *>(&server_address),
            sizeof(server_address));

    if (bind_result < 0) {
        throw runtime_error("Cannot bind socket.");
    }

    struct timeval udp_socket_timeout;
    udp_socket_timeout.tv_sec = 0;
    udp_socket_timeout.tv_usec = usec_timeout;

    setsockopt(
            socket_fd_,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (const char*)&udp_socket_timeout,
            sizeof(struct timeval));
}

int UdpReceiver::receive_many(mmsghdr* msgs, const size_t n_msgs)
{
    return recvmmsg(socket_fd_, msgs, n_msgs, 0, 0);
}

bool UdpReceiver::receive(void* buffer, size_t buffer_n_bytes)
{
    auto data_len = recv(socket_fd_, buffer, buffer_n_bytes, 0);

    if (data_len < 0) {
        return false;
    }

    if (data_len != buffer_n_bytes) {
        #ifdef DEBUG_OUTPUT
            using namespace date;
            using namespace chrono;
            cout << "[" << system_clock::now() << "]";
            cout << "[UdpReceiver::receive]";
            cout << " Expected buffer_n_bytes " << buffer_n_bytes;
            cout << " but got data_len " << data_len << endl;
        #endif
        return false;
    }

    return true;
}

void UdpReceiver::disconnect()
{
    close(socket_fd_);
    socket_fd_ = -1;
}
