#include <memory>

#include "JFH5Writer.hpp"
#include "gtest/gtest.h"
#include "bitshuffle/bitshuffle.h"

using namespace std;
using namespace buffer_config;

TEST(JFH5Writer, basic_interaction)
{
    size_t n_modules = 2;
    uint64_t start_pulse_id = 1;
    uint64_t stop_pulse_id = 5;

    auto data = make_unique<char[]>(n_modules*MODULE_N_BYTES*BUFFER_BLOCK_SIZE);
    auto metadata = make_shared<ImageMetadataBlock>();

    // Needed by writer.
    metadata->block_start_pulse_id = 0;
    metadata->block_stop_pulse_id = BUFFER_BLOCK_SIZE - 1;

    JFH5Writer writer("ignore.h5", start_pulse_id, stop_pulse_id, n_modules);
    writer.write(metadata.get(), data.get());
}

TEST(JFH5Writer, test_writing)
{
    size_t n_modules = 2;
    uint64_t start_pulse_id = 5;
    uint64_t stop_pulse_id = 10;
    auto n_images = stop_pulse_id - start_pulse_id + 1;

    auto metadata = make_shared<ImageMetadataBlock>();
    metadata->block_start_pulse_id = 0;
    metadata->block_stop_pulse_id = BUFFER_BLOCK_SIZE - 1;

    for (uint64_t pulse_id=start_pulse_id;
         pulse_id<=stop_pulse_id;
         pulse_id++) {

        metadata->pulse_id[pulse_id] = pulse_id;
        metadata->frame_index[pulse_id] = pulse_id + 10;
        metadata->daq_rec[pulse_id] = pulse_id + 100;
        metadata->is_good_image[pulse_id] = 1;
    }


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

    // The writer closes the file on destruction.
    {
        JFH5Writer writer(
                "ignore.h5", start_pulse_id, stop_pulse_id, n_modules);
        writer.write(metadata.get(), (char*)(&image_buffer[0]));
    }

    H5::H5File reader("ignore.h5", H5F_ACC_RDONLY);
    auto image_dataset = reader.openDataSet("image");
    image_dataset.read(&image_buffer[0], H5::PredType::NATIVE_UINT16);

    for (int i_image=0; i_image < n_images; i_image++) {
        for (int i_module=0; i_module<n_modules; i_module++) {

            auto offset = i_image * MODULE_N_PIXELS;
            offset += i_module * MODULE_N_PIXELS;

            for (int i_pixel=0; i_pixel<MODULE_N_PIXELS; i_pixel++) {
                ASSERT_EQ(image_buffer[offset + i_pixel], i_pixel % 100);
            }
        }
    }

    auto pulse_id_data = make_unique<uint64_t[]>(n_images);
    auto pulse_id_dataset = reader.openDataSet("pulse_id");
    pulse_id_dataset.read(&pulse_id_data[0], H5::PredType::NATIVE_UINT64);

    auto frame_index_data = make_unique<uint64_t[]>(n_images);
    auto frame_index_dataset = reader.openDataSet("frame_index");
    frame_index_dataset.read(&frame_index_data[0], H5::PredType::NATIVE_UINT64);

    auto daq_rec_data = make_unique<uint32_t[]>(n_images);
    auto daq_rec_dataset = reader.openDataSet("daq_rec");
    daq_rec_dataset.read(&daq_rec_data[0], H5::PredType::NATIVE_UINT32);

    auto is_good_frame_data = make_unique<uint8_t[]>(n_images);
    auto is_good_frame_dataset = reader.openDataSet("is_good_frame");
    is_good_frame_dataset.read(
            &is_good_frame_data[0], H5::PredType::NATIVE_UINT8);

    for (uint64_t pulse_id=start_pulse_id;
         pulse_id<=stop_pulse_id;
         pulse_id++) {

        ASSERT_EQ(pulse_id_data[pulse_id - start_pulse_id], pulse_id);
        ASSERT_EQ(frame_index_data[pulse_id - start_pulse_id], pulse_id + 10);
        ASSERT_EQ(daq_rec_data[pulse_id - start_pulse_id], pulse_id + 100);
        ASSERT_EQ(is_good_frame_data[pulse_id - start_pulse_id], 1);
    }
}