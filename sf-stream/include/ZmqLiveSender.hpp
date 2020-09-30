#ifndef SF_DAQ_BUFFER_ZMQLIVESENDER_HPP
#define SF_DAQ_BUFFER_ZMQLIVESENDER_HPP

#include <string>
#include "formats.hpp"
#include "BufferUtils.hpp"


class ZmqLiveSender {
    const void* ctx_;
    const BufferUtils::DetectorConfig config_;

    void* socket_streamvis_;
    void* socket_live_;
    void* socket_pulse_;

public:
    ZmqLiveSender(void* ctx,
                  const BufferUtils::DetectorConfig& config);
    ~ZmqLiveSender();

    void send(const ImageMetadata& meta, const char* data);
};


#endif //SF_DAQ_BUFFER_ZMQLIVESENDER_HPP
