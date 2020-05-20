#ifndef SF_DAQ_BUFFER_REPLAYH5READER_HPP
#define SF_DAQ_BUFFER_REPLAYH5READER_HPP

#include <string>
#include "jungfrau.hpp"
#include <H5Cpp.h>
#include <memory>
#include "buffer_config.hpp"


class ReplayH5Reader {

    const std::string device_;
    const std::string channel_name_;
    const uint64_t stop_pulse_id_;

    H5::H5File current_file_;
    std::string current_filename_;
    H5::DataSet dset_metadata_;
    H5::DataSet dset_frame_;

    void load_buffers(const uint64_t pulse_id,
                      const size_t n_pulses,
                      ReplayModuleFrameBuffer* metadata,
                      char* frame_buffer);

public:
    ReplayH5Reader(
            const std::string device,
            const std::string channel_name,
            const uint64_t stop_pulse_id);
    virtual ~ReplayH5Reader();

    void close_file();
    bool get_buffer(
            const uint64_t pulse_id,
            ReplayModuleFrameBuffer* metadata,
            char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_REPLAYH5READER_HPP