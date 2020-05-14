#ifndef SF_DAQ_BUFFER_REPLAYH5READER_HPP
#define SF_DAQ_BUFFER_REPLAYH5READER_HPP

#include <string>
#include "jungfrau.hpp"
#include <H5Cpp.h>


class ReplayH5Reader {

    const std::string device_;
    const std::string channel_name_;

    H5::H5File current_file_;
    std::string current_filename_;
    H5::DataSet dset_metadata_;
    H5::DataSet dset_frame_;

    void prepare_file_for_pulse(const uint64_t pulse_id);

public:
    ReplayH5Reader(const std::string device, const std::string channel_name);
    virtual ~ReplayH5Reader();
    void close_file();
    bool get_frame(
            const uint64_t pulse_id, ModuleFrame* metadata, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_REPLAYH5READER_HPP
