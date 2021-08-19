#ifndef SF_DAQ_BUFFER_ZMQLIVESENDER_HPP
#define SF_DAQ_BUFFER_ZMQLIVESENDER_HPP

#include <string>
#include "formats.hpp"
#include "BufferUtils.hpp"


class ZmqLiveSender {
    void* ctx_;
    const std::string& det_name_;
    const std::string& stream_address_;

    void* socket_streamvis_;

private:
    std::string _get_data_type_mapping(int dtype) const;

public:
    ZmqLiveSender(void* ctx,
                  const std::string& det_name,
                  const std::string& stream_address);
    ~ZmqLiveSender();

    void send(const ImageMetadata& meta, const char* data,
            const size_t image_n_bytes);

};


#endif //SF_DAQ_BUFFER_ZMQLIVESENDER_HPP
