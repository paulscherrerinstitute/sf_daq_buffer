#ifndef SF_DAQ_BUFFER_LIVEH5READER_HPP
#define SF_DAQ_BUFFER_LIVEH5READER_HPP

#include <string>
#include "jungfrau.hpp"

class LiveH5Reader {

    const std::string current_filename_;
    const uint16_t source_id_;

public:
    LiveH5Reader(
            const std::string& device,
            const std::string& channel_name,
            const uint16_t source_id);

    uint64_t get_latest_pulse_id();
    ModuleFrame* read_frame_metadata(uint64_t pulse_id);
    char* read_frame_data(uint64_t pulse_id);
};


#endif //SF_DAQ_BUFFER_LIVEH5READER_HPP
