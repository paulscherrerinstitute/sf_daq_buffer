#ifndef SF_DAQ_BUFFER_FORMATS_HPP
#define SF_DAQ_BUFFER_FORMATS_HPP

#include "buffer_config.hpp"
#include "jungfrau.hpp"

struct ImageMetadataBuffer
{
    uint64_t pulse_id[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint64_t frame_index[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint32_t daq_rec[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint8_t is_good_image[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint16_t n_images;
};

const char BUFFER_FORMAT_START_BYTE = 0xBE;

#pragma pack(push)
#pragma pack(1)
struct BufferBinaryFormat {

    BufferBinaryFormat() : FORMAT_MARKER(BUFFER_FORMAT_START_BYTE) {};

    const char FORMAT_MARKER;
    ModuleFrame metadata;
    char data[core_buffer::MODULE_N_BYTES];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct BufferBlock
{
    BufferBinaryFormat frame[core_buffer::BUFFER_BLOCK_SIZE];
    uint64_t start_pulse_id;
};
#pragma pack(pop)

#endif //SF_DAQ_BUFFER_FORMATS_HPP
