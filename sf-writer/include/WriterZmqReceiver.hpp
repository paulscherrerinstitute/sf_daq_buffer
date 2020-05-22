#ifndef SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP
#define SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP

#include <string>
#include "WriterH5Writer.hpp"
#include <vector>
#include <jungfrau.hpp>


class WriterZmqReceiver {

    const size_t n_modules_;
    std::vector<void*> sockets_;

    ReplayModuleFrameBuffer frame_metadata;

public:
    WriterZmqReceiver(
            void *ctx,
            const std::string& ipc_prefix,
            const size_t n_modules);

    virtual ~WriterZmqReceiver();

    void get_next_batch(
            const uint64_t pulse_id,
            ImageMetadataBuffer* image_metadata,
            char* image_buffer);
};


#endif //SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP
