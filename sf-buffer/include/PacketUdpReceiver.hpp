#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <sys/socket.h>

class PacketUdpReceiver {

    int socket_fd_;

public:
    PacketUdpReceiver();
    virtual ~PacketUdpReceiver();

    bool receive(void* buffer, const size_t buffer_n_bytes);
    int receive_many(mmsghdr* msgs, const size_t n_msgs);

    void bind(const uint16_t port);
    void disconnect();
};


#endif //LIB_CPP_H5_WRITER_UDPRECEIVER_H
