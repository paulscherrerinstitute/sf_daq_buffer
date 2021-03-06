#include <memory>

#include "JFH5Writer.hpp"
#include "gtest/gtest.h"
#include "bitshuffle/bitshuffle.h"
#include "mock/data.hpp"

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

    JFH5Writer writer("ignore.h5", "detector",
            n_modules, start_pulse_id, stop_pulse_id, 1);
    writer.write(metadata.get(), data.get());
}

TEST(JFH5Writer, test_writing)
{
    size_t n_modules = 2;
    uint64_t start_pulse_id = 5;
    uint64_t stop_pulse_id = 10;
    auto n_images = stop_pulse_id - start_pulse_id + 1;

    auto meta = get_test_block_metadata(start_pulse_id, stop_pulse_id, 1);
    auto data = get_test_block_data(n_modules);

    string detector_name = "detector";

    // The writer closes the file on destruction.
    {
        JFH5Writer writer(
                "ignore.h5", detector_name,
                n_modules, start_pulse_id, stop_pulse_id, 1);
        writer.write(meta.get(), (char*)(&data[0]));
    }

    H5::H5File reader("ignore.h5", H5F_ACC_RDONLY);
    auto image_dataset = reader.openDataSet("/data/detector/data");
    image_dataset.read(&data[0], H5::PredType::NATIVE_UINT16);

    for (int i_image=0; i_image < n_images; i_image++) {
        for (int i_module=0; i_module<n_modules; i_module++) {

            auto offset = i_image * MODULE_N_PIXELS;
            offset += i_module * MODULE_N_PIXELS;

            for (int i_pixel=0; i_pixel<MODULE_N_PIXELS; i_pixel++) {
                ASSERT_EQ(data[offset + i_pixel], i_pixel % 100);
            }
        }
    }

    auto pulse_id_data = make_unique<uint64_t[]>(n_images);
    auto pulse_id_dataset = reader.openDataSet("/data/detector/pulse_id");
    pulse_id_dataset.read(&pulse_id_data[0], H5::PredType::NATIVE_UINT64);

    auto frame_index_data = make_unique<uint64_t[]>(n_images);
    auto frame_index_dataset = reader.openDataSet("/data/detector/frame_index");
    frame_index_dataset.read(&frame_index_data[0], H5::PredType::NATIVE_UINT64);

    auto daq_rec_data = make_unique<uint32_t[]>(n_images);
    auto daq_rec_dataset = reader.openDataSet("/data/detector/daq_rec");
    daq_rec_dataset.read(&daq_rec_data[0], H5::PredType::NATIVE_UINT32);

    auto is_good_frame_data = make_unique<uint8_t[]>(n_images);
    auto is_good_frame_dataset =
            reader.openDataSet("/data/detector/is_good_frame");
    is_good_frame_dataset.read(
            &is_good_frame_data[0], H5::PredType::NATIVE_UINT8);

    auto name_dataset = reader.openDataSet("/general/detector_name");
    string read_detector_name;
    name_dataset.read(read_detector_name, name_dataset.getDataType());

    ASSERT_EQ(detector_name, read_detector_name);

    for (uint64_t pulse_id=start_pulse_id;
         pulse_id<=stop_pulse_id;
         pulse_id++) {

        ASSERT_EQ(pulse_id_data[pulse_id - start_pulse_id], pulse_id);
        ASSERT_EQ(frame_index_data[pulse_id - start_pulse_id], pulse_id + 10);
        ASSERT_EQ(daq_rec_data[pulse_id - start_pulse_id], pulse_id + 100);
        ASSERT_EQ(is_good_frame_data[pulse_id - start_pulse_id], 1);
    }
}

TEST(JFH5Writer, test_step_pulse_id)
{
    // Start pulse id (5) larger than stop pulse id (4).
    ASSERT_THROW(JFH5Writer writer("ignore.h5", "d", 1 , 5, 4, 1),
            runtime_error);

    // Start pulse id (5) is equal to stop pulse id (5).
    ASSERT_NO_THROW(JFH5Writer writer("ignore.h5", "d", 1, 5, 5, 1));

    // The step is exactly on start nad stop pulse id.
    ASSERT_NO_THROW(JFH5Writer writer("ignore.h5", "d", 1, 5, 5, 5));

    // No pulses in given range with step = 10
    ASSERT_THROW(JFH5Writer writer("ignore.h5", "d", 1, 1, 9, 10),
            runtime_error);

    // Stop pulse id is divisible by step, but start is not.
    ASSERT_THROW(JFH5Writer writer("ignore.h5", "d", 1, 5, 10, 10),
            runtime_error);

    // Start pulse id is divisible by step, but stop is not.
    ASSERT_THROW(JFH5Writer writer("ignore.h5", "d", 1, 10, 19, 10),
            runtime_error);

    // Should be ok.
    ASSERT_NO_THROW(JFH5Writer("ignore.h5", "d", 1, 1234, 1234, 1));
    // Should be ok.
    ASSERT_NO_THROW(JFH5Writer("ignore.h5", "d", 1, 1234, 4567, 1));
    // Should be ok.
    ASSERT_NO_THROW(JFH5Writer("ignore.h5", "d", 1, 4, 4, 4));

    // stop smaller than start.
    ASSERT_THROW(JFH5Writer("ignore.h5", "d", 1, 1234, 1233, 1),
            runtime_error);
    // step is not valid for 100Hz.
    ASSERT_THROW(JFH5Writer("ignore.h5", "d", 1, 1234, 1234, 3),
            runtime_error);
    // start not divisible by step.
    ASSERT_THROW(JFH5Writer("ignore.h5", "d", 1, 10, 10, 4),
            runtime_error);
    // stop not divisible by step
    ASSERT_THROW(JFH5Writer("ignore.h5", "d", 1, 8, 10, 4),
            runtime_error);
}

void test_writing_with_step(
        uint64_t start_pulse_id, uint64_t stop_pulse_id, size_t step)
{
    size_t n_modules = 3;

    size_t n_images = 1;
    n_images += (stop_pulse_id / step);
    n_images -= start_pulse_id / step;

    auto meta = get_test_block_metadata(start_pulse_id, stop_pulse_id, step);
    auto data = get_test_block_data(n_modules);

    // Verify the metadata has the layout we want to test (50Hz).
    for (size_t i_pulse=0; i_pulse<BUFFER_BLOCK_SIZE; i_pulse++) {
        if (i_pulse % step == 0) {
            ASSERT_EQ(meta->pulse_id[i_pulse], 500 + i_pulse);
        } else {
            ASSERT_EQ(meta->pulse_id[i_pulse], 0);
        }
    }

    string path_root = "/path/to/";
    string expected_detector_name = "detector";

    // The writer closes the file on destruction.
    {
        JFH5Writer writer(
                "ignore.h5", path_root + expected_detector_name,
                n_modules, start_pulse_id, stop_pulse_id, step);
        writer.write(meta.get(), (char*)(&data[0]));
    }

    H5::H5File reader("ignore.h5", H5F_ACC_RDONLY);
    auto image_dataset = reader.openDataSet("/data/detector/data");
    image_dataset.read(&data[0], H5::PredType::NATIVE_UINT16);

    hsize_t dims[3];
    image_dataset.getSpace().getSimpleExtentDims(dims);
    ASSERT_EQ(dims[0], n_images);
    ASSERT_EQ(dims[1], n_modules * MODULE_Y_SIZE);
    ASSERT_EQ(dims[2], MODULE_X_SIZE);

    auto pulse_id_data = make_unique<uint64_t[]>(n_images);
    auto pulse_id_dataset = reader.openDataSet("/data/detector/pulse_id");
    pulse_id_dataset.read(&pulse_id_data[0], H5::PredType::NATIVE_UINT64);
    pulse_id_dataset.getSpace().getSimpleExtentDims(dims);
    ASSERT_EQ(dims[0], n_images);
    ASSERT_EQ(dims[1], 1);

    auto frame_index_data = make_unique<uint64_t[]>(n_images);
    auto frame_index_dataset = reader.openDataSet("/data/detector/frame_index");
    frame_index_dataset.read(&frame_index_data[0], H5::PredType::NATIVE_UINT64);
    frame_index_dataset.getSpace().getSimpleExtentDims(dims);
    ASSERT_EQ(dims[0], n_images);
    ASSERT_EQ(dims[1], 1);

    auto daq_rec_data = make_unique<uint32_t[]>(n_images);
    auto daq_rec_dataset = reader.openDataSet("/data/detector/daq_rec");
    daq_rec_dataset.read(&daq_rec_data[0], H5::PredType::NATIVE_UINT32);
    daq_rec_dataset.getSpace().getSimpleExtentDims(dims);
    ASSERT_EQ(dims[0], n_images);
    ASSERT_EQ(dims[1], 1);

    auto is_good_frame_data = make_unique<uint8_t[]>(n_images);
    auto is_good_frame_dataset =
            reader.openDataSet("/data/detector/is_good_frame");
    is_good_frame_dataset.read(
            &is_good_frame_data[0], H5::PredType::NATIVE_UINT8);
    is_good_frame_dataset.getSpace().getSimpleExtentDims(dims);
    ASSERT_EQ(dims[0], n_images);
    ASSERT_EQ(dims[1], 1);

    auto name_dataset = reader.openDataSet("/general/detector_name");
    string read_detector_name;
    name_dataset.read(read_detector_name, name_dataset.getDataType());

    ASSERT_EQ(expected_detector_name, read_detector_name);

    uint64_t i_pulse = 0;
    for (uint64_t pulse_id=start_pulse_id;
         pulse_id<=stop_pulse_id;
         pulse_id++) {

        if (pulse_id % step != 0) {
            continue;
        }

        ASSERT_EQ(pulse_id_data[i_pulse], pulse_id);
        ASSERT_EQ(frame_index_data[i_pulse], pulse_id + 10);
        ASSERT_EQ(daq_rec_data[i_pulse], pulse_id + 100);
        ASSERT_EQ(is_good_frame_data[i_pulse], 1);

        i_pulse++;
    }
}

TEST(JFH5Writer, test_writing_with_step)
{
    // TODO: Write with any number of steps.
    // 100Hz
    test_writing_with_step(500, 599, 1);
    // 50Hz
    test_writing_with_step(500, 598, 2);
    // 25Hz
    test_writing_with_step(500, 596, 4);
    // 10Hz
    test_writing_with_step(500, 590, 10);
    // 1Hz
    test_writing_with_step(500, 500, 100);
}
