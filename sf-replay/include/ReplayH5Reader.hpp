#ifndef SF_DAQ_BUFFER_REPLAYH5READER_HPP
#define SF_DAQ_BUFFER_REPLAYH5READER_HPP

#include <string>
#include <H5Cpp.h>
#include <memory>

#include "formats.hpp"

class ReplayH5Reader {

    const std::string device_;
    const std::string channel_name_;

    H5::H5File current_file_;
    std::string current_filename_;
    H5::DataSet dset_metadata_;
    H5::DataSet dset_frame_;

public:
    ReplayH5Reader(
            const std::string device,
            const std::string channel_name);
    virtual ~ReplayH5Reader();

    void close_file();
    void get_buffer(
            const uint64_t pulse_id,
            ReplayBuffer* metadata,
            char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_REPLAYH5READER_HPP
