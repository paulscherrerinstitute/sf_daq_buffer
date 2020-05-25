#ifndef SF_DAQ_BUFFER_FORMATS_HPP
#define SF_DAQ_BUFFER_FORMATS_HPP

#include "buffer_config.hpp"
#include "jungfrau.hpp"

struct ImageMetadataBuffer
{
    uint64_t pulse_id[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint64_t frame_index[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint32_t daq_rec[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint8_t is_good_frame[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint16_t n_images;
};

#pragma pack(push)
#pragma pack(1)
struct ReplayModuleFrameBuffer {
    ModuleFrame frame[core_buffer::REPLAY_READ_BUFFER_SIZE];
    bool is_frame_present[core_buffer::REPLAY_READ_BUFFER_SIZE];
    bool is_good_frame[core_buffer::REPLAY_READ_BUFFER_SIZE];
    uint16_t module_id;
    uint64_t data_n_bytes;
    uint16_t n_frames;
};
#pragma pack(pop)

#endif //SF_DAQ_BUFFER_FORMATS_HPP
