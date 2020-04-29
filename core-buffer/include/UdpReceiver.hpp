#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <sys/socket.h>
#include "buffer_config.hpp"

class UdpReceiver {

    int socket_fd_;

public:
    UdpReceiver();
    virtual ~UdpReceiver();

    bool receive(void* buffer, size_t buffer_n_bytes);
    int receive_many(mmsghdr* msgs, const size_t n_msgs);

    void bind(
            const uint16_t port,
            const size_t usec_timeout=core_buffer::BUFFER_UDP_US_TIMEOUT);
    void disconnect();
};


#endif //LIB_CPP_H5_WRITER_UDPRECEIVER_H
