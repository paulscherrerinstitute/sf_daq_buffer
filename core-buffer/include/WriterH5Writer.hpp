#ifndef SFWRITER_HPP
#define SFWRITER_HPP

#include <memory>
#include <string>
#include <H5Cpp.h>
#include "buffer_config.hpp"

// Size of compression header in bytes.
const int BSHUF_LZ4_HEADER_BYTES = 12;

struct ImageMetadata
{
    uint64_t pulse_id;
    uint64_t frame_index;
    uint32_t daq_rec;
    uint8_t is_good_frame;
    uint64_t compressed_image_size;
};

class WriterH5Writer {

    const size_t n_frames_;
    size_t current_write_index_;

    H5::H5File file_;

    H5::DataSet image_dataset_;
    H5::DataSet pulse_id_dataset_;
    H5::DataSet frame_index_dataset_;
    H5::DataSet daq_rec_dataset_;
    H5::DataSet is_good_frame_dataset_;


public:
    WriterH5Writer(
            const std::string& output_file,
            const size_t n_frames,
            const size_t n_modules);
    ~WriterH5Writer();
    void write(const ImageMetadata* metadata, const char* data);
    void close_file();
};


#endif //SFWRITER_HPP
