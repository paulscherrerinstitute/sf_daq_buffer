#ifndef SF_DAQ_BUFFER_LIVEH5READER_HPP
#define SF_DAQ_BUFFER_LIVEH5READER_HPP

#include <string>
#include <memory>
#include "jungfrau.hpp"
#include "buffer_config.hpp"

class LiveH5Reader {

    struct LiveBufferMetadata {
        uint64_t pulse_id[core_buffer::FILE_MOD];
        uint64_t frame_index[core_buffer::FILE_MOD];
        uint32_t daq_rec[core_buffer::FILE_MOD];
        uint16_t n_received_packets[core_buffer::FILE_MOD];
    };

    const std::string current_filename_;
    const uint16_t source_id_;
    std::unique_ptr<LiveBufferMetadata> metadata_buffer_;
    std::unique_ptr<uint16_t[]> data_buffer_;

public:
    LiveH5Reader(
            const std::string& device,
            const std::string& channel_name,
            const uint16_t source_id);

    uint64_t get_latest_pulse_id();
    void load_pulse_id(uint64_t pulse_id);

    ModuleFrame* get_metadata();
    char* get_data();
};


#endif //SF_DAQ_BUFFER_LIVEH5READER_HPP
