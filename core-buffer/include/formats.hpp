#ifndef SF_DAQ_BUFFER_FORMATS_HPP
#define SF_DAQ_BUFFER_FORMATS_HPP

#include "buffer_config.hpp"


#pragma pack(push)
#pragma pack(1)
struct ModuleFrame {
    uint64_t id;
    uint64_t pulse_id;
    uint64_t frame_index;
    uint64_t daq_rec;
    uint64_t n_recv_packets;
    uint64_t module_id;
    uint16_t bit_depth;
    uint16_t pos_y;
    uint16_t pos_x;
};
#pragma pack(pop)


#pragma pack(push)
#pragma pack(1)
struct ImageMetadata {
    uint64_t pulse_id;
    uint64_t frame_index;
    uint32_t daq_rec;
    uint32_t is_good_image;
};
#pragma pack(pop)


#endif //SF_DAQ_BUFFER_FORMATS_HPP
