#ifndef SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP
#define SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP

#include <string>
#include "WriterH5Writer.hpp"
#include <vector>
#include <jungfrau.hpp>


class WriterZmqReceiver {

    const size_t n_modules_;
    std::vector<void*> sockets_;
    const uint64_t end_pulse_id_;

    ReplayModuleFrameBuffer frame_metadata;

public:
    WriterZmqReceiver(
            void *ctx,
            const std::string& ipc_prefix,
            const size_t n_modules,
            const uint64_t stop_pulse_id);

    virtual ~WriterZmqReceiver();

    void get_next_buffer(
            const uint64_t start_pulse_id,
            ImageMetadataBuffer* image_metadata,
            char* image_buffer);
};


#endif //SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP
