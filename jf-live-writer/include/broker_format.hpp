#ifndef SF_DAQ_BUFFER_BROKER_FORMAT_HPP
#define SF_DAQ_BUFFER_BROKER_FORMAT_HPP

#include "formats.hpp"

const static uint8_t OP_START = 1;
const static uint8_t OP_END = 2;

#pragma pack(push)
#pragma pack(1)
struct StoreStream {
    ImageMetadata image_metadata;

    int64_t run_id;
    uint32_t i_image;
    uint32_t n_images;
    uint32_t image_y_size;
    uint32_t image_x_size;
    uint32_t op_code;
    uint32_t bits_per_pixel;
};
#pragma pack(pop)
#endif //SF_DAQ_BUFFER_BROKER_FORMAT_HPP
