#ifndef SF_DAQ_BUFFER_LIVEH5READER_HPP
#define SF_DAQ_BUFFER_LIVEH5READER_HPP

#include <string>
#include <memory>
#include "jungfrau.hpp"
#include "buffer_config.hpp"

class LiveH5Reader {

    const std::string current_filename_;
    const uint16_t source_id_;

    std::unique_ptr<uint64_t[]> pulse_id_buffer_;
    std::unique_ptr<uint16_t[]> data_buffer_;

public:
    LiveH5Reader(
            const std::string& device,
            const std::string& channel_name,
            const uint16_t source_id);

    uint64_t get_latest_pulse_id();
    void load_pulse_id(uint64_t pulse_id);

    ModuleFrame get_metadata();
    char* get_data();
};


#endif //SF_DAQ_BUFFER_LIVEH5READER_HPP
