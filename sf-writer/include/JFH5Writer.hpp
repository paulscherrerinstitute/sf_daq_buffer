#ifndef SFWRITER_HPP
#define SFWRITER_HPP

#include <memory>
#include <string>
#include <H5Cpp.h>
#include "buffer_config.hpp"
#include "formats.hpp"

class JFH5Writer {

    const size_t n_frames_;
    const size_t n_modules_;
    size_t current_write_index_;

    H5::H5File file_;

    H5::DataSet image_dataset_;
    H5::DataSet pulse_id_dataset_;
    H5::DataSet frame_index_dataset_;
    H5::DataSet daq_rec_dataset_;
    H5::DataSet is_good_frame_dataset_;


public:
    JFH5Writer(const std::string& output_file,
               const size_t n_frames,
               const size_t n_modules);
    ~JFH5Writer();
    void write(const ImageMetadataBuffer* metadata, const char* data);
    void close_file();

    uint64_t* b_pulse_id_;
    uint64_t* b_frame_index_;
    uint32_t* b_daq_rec_;
    uint8_t* b_is_good_frame_ ;
};


#endif //SFWRITER_HPP
