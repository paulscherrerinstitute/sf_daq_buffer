#ifndef SF_DAQ_BUFFER_BROKER_FORMAT_HPP
#define SF_DAQ_BUFFER_BROKER_FORMAT_HPP

#include "formats.hpp"

const static uint8_t OP_START = 1;
const static uint8_t OP_END = 2;

#pragma pack(push)
#pragma pack(1)
struct StoreStream {
    uint8_t op_code;
    uint32_t i_image;
    uint32_t n_images;
    int64_t run_id;

    ImageMetadata image_metadata;
};
#pragma pack(pop)
#endif //SF_DAQ_BUFFER_BROKER_FORMAT_HPP
