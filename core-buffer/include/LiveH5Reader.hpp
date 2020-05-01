#ifndef SF_DAQ_BUFFER_LIVEH5READER_HPP
#define SF_DAQ_BUFFER_LIVEH5READER_HPP

#include <string>
#include <memory>
#include "jungfrau.hpp"
#include "buffer_config.hpp"
#include <H5Cpp.h>

class LiveH5Reader {

    const std::string current_filename_;
    const uint16_t source_id_;

    std::unique_ptr<uint64_t[]> pulse_id_buffer_;
    std::unique_ptr<uint16_t[]> data_buffer_;

    uint64_t current_file_max_pulse_id_;
    H5::H5File file_;

    H5::DataSet image_dataset_;
    H5::DataSet pulse_id_dataset_;
    H5::DataSet frame_index_dataset_;
    H5::DataSet daq_rec_dataset_;
    H5::DataSet n_received_packets_dataset_;

    void open_file();

public:
    LiveH5Reader(
            const std::string& device,
            const std::string& channel_name,
            const uint16_t source_id);

    ~LiveH5Reader();

    uint64_t get_latest_pulse_id();
    void load_pulse_id(uint64_t pulse_id);

    ModuleFrame get_metadata();
    char* get_data();

    void close_file();
};


#endif //SF_DAQ_BUFFER_LIVEH5READER_HPP
