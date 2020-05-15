#ifndef SFWRITER_HPP
#define SFWRITER_HPP

#include <memory>
#include <string>
#include <H5Cpp.h>
#include "buffer_config.hpp"

struct DetectorFrame
{
    uint64_t pulse_id[core_buffer::WRITER_N_FRAMES_BUFFER];
    uint64_t frame_index[core_buffer::WRITER_N_FRAMES_BUFFER];
    uint32_t daq_rec[core_buffer::WRITER_N_FRAMES_BUFFER];
    uint16_t n_received_packets[core_buffer::WRITER_N_FRAMES_BUFFER];
    bool     is_good_frame[core_buffer::WRITER_N_FRAMES_BUFFER];
};

class WriterH5Writer {

    const size_t n_frames_;
    const size_t n_modules_;
    size_t current_write_index_;

    H5::H5File file_;

    H5::DataSet image_dataset_;
    H5::DataSet pulse_id_dataset_;
    H5::DataSet frame_index_dataset_;
    H5::DataSet daq_rec_dataset_;
    H5::DataSet n_received_packets_dataset_;

public:
    WriterH5Writer(
            const std::string& output_file,
            const size_t n_frames,
            const size_t n_modules);
    ~WriterH5Writer();
    void write(const DetectorFrame* metadata, const char* data);
    void close_file();
};


#endif //SFWRITER_HPP
