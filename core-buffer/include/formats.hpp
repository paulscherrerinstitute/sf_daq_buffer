#ifndef SF_DAQ_BUFFER_FORMATS_HPP
#define SF_DAQ_BUFFER_FORMATS_HPP

#include "buffer_config.hpp"

struct ImageMetadataBuffer
{
    uint64_t pulse_id[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint64_t frame_index[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint32_t daq_rec[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint8_t is_good_frame[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint64_t data_n_bytes[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint16_t n_images;
};

struct ImageMetadata
{
    uint64_t pulse_id;
    uint64_t frame_index;
    uint32_t daq_rec;
    uint8_t is_good_frame;
    uint64_t data_n_bytes;
};

#endif //SF_DAQ_BUFFER_FORMATS_HPP
