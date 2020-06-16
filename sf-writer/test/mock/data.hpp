#ifndef SF_DAQ_BUFFER_DATA_HPP
#define SF_DAQ_BUFFER_DATA_HPP

#include <utility>
#include <memory>

#include "buffer_config.hpp"

auto get_test_block_metadata(
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id,
        const int pulse_id_step)
{
    using namespace std;
    using namespace buffer_config;

    auto metadata = make_shared<ImageMetadataBlock>();
    metadata->block_start_pulse_id = start_pulse_id;
    metadata->block_stop_pulse_id = start_pulse_id + BUFFER_BLOCK_SIZE - 1;

    for (uint64_t pulse_id = start_pulse_id;
         pulse_id <= stop_pulse_id;
         pulse_id++) {

        if (pulse_id % pulse_id_step != 0) {
            metadata->is_good_image[pulse_id] = 0;
            continue;
        }

        metadata->pulse_id[pulse_id] = pulse_id;
        metadata->frame_index[pulse_id] = pulse_id + 10;
        metadata->daq_rec[pulse_id] = pulse_id + 100;
        metadata->is_good_image[pulse_id] = 1;
    }

    return metadata;
}

auto get_test_block_data(const size_t n_modules)
{
    using namespace std;
    using namespace buffer_config;

    auto image_buffer = make_unique<uint16_t[]>(
            MODULE_N_PIXELS * n_modules * BUFFER_BLOCK_SIZE);

    for (int i_block=0; i_block<=BUFFER_BLOCK_SIZE; i_block++) {
        for (int i_module=0; i_module<n_modules; i_module++) {
            auto offset = i_block * MODULE_N_PIXELS;
            offset += i_module * MODULE_N_PIXELS;

            for (int i_pixel=0; i_pixel<MODULE_N_PIXELS; i_pixel++) {
                image_buffer[offset + i_pixel] = i_pixel % 100;
            }
        }
    }

    return image_buffer;
}


#endif //SF_DAQ_BUFFER_DATA_HPP
