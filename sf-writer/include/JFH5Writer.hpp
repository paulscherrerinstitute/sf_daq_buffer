#ifndef SFWRITER_HPP
#define SFWRITER_HPP

#include <memory>
#include <string>
#include <H5Cpp.h>

#include "ImageAssembler.hpp"

class JFH5Writer {

    const size_t n_modules_;
    const uint64_t start_pulse_id_;
    const uint64_t stop_pulse_id_;
    const int pulse_id_step_;
    const size_t n_images_;
    size_t meta_write_index_;
    size_t data_write_index_;

    H5::H5File file_;
    H5::DataSet image_dataset_;

    uint64_t* b_pulse_id_;
    uint64_t* b_frame_index_;
    uint32_t* b_daq_rec_;
    uint8_t* b_is_good_frame_ ;

    size_t get_n_pulses_in_range(const uint64_t start_pulse_id,
                                 const uint64_t stop_pulse_id,
                                 const int pulse_id_step);
    void close_file();

public:
    JFH5Writer(const std::string& output_file,
               const size_t n_modules,
               const uint64_t start_pulse_id,
               const uint64_t stop_pulse_id,
               const int pulse_id_step);
    ~JFH5Writer();
    void write(const ImageMetadataBlock* metadata, const char* data);
};

#endif //SFWRITER_HPP
