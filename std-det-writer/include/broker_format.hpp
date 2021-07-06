#ifndef SF_DAQ_BUFFER_BROKER_FORMAT_HPP
#define SF_DAQ_BUFFER_BROKER_FORMAT_HPP

#include "formats.hpp"


#pragma pack(push)
#pragma pack(1)
struct StoreStream {
    std::string output_file;
    int64_t run_id;
    uint64_t image_id;
    uint32_t i_image;
    uint32_t n_images;
};
#pragma pack(pop)
#endif //SF_DAQ_BUFFER_BROKER_FORMAT_HPP
