#include "BufferH5Writer.hpp"
#include "gtest/gtest.h"

using namespace core_buffer;

TEST(BufferH5Writer, basic_interaction)
{
    auto root_folder = ".";
    auto device_name = "fast_device";
    size_t pulse_id = 1;

    auto buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);

    ModuleFrame metadata;
    metadata.pulse_id = 1;
    metadata.frame_index = 2;
    metadata.daq_rec = 3;
    metadata.n_received_packets = 128;

    BufferH5Writer writer(device_name, root_folder);
    writer.set_pulse_id(pulse_id);
    writer.write(&metadata, buffer.get());
    writer.close_file();

    auto filename = BufferUtils::get_filename(
            root_folder, device_name, pulse_id);

    auto file_frame_index = BufferUtils::get_file_frame_index(pulse_id);

    H5::H5File input_file(filename, H5F_ACC_RDONLY);

    auto image_dataset = input_file.openDataSet("image");
    auto image_buffer = make_unique<uint16_t[]>(1000*512*1024);
    image_dataset.read(image_buffer.get(), H5::PredType::NATIVE_UINT16);

    auto metadata_dataset = input_file.openDataSet("metadata");
    auto metadata_buffer = make_unique<ModuleFrame[]>(FILE_MOD);
    metadata_dataset.read(metadata_buffer.get(), H5::PredType::NATIVE_UINT64);

    EXPECT_EQ(metadata_buffer[file_frame_index].pulse_id, 1);
    EXPECT_EQ(metadata_buffer[file_frame_index].frame_index, 2);
    EXPECT_EQ(metadata_buffer[file_frame_index].daq_rec, 3);
    EXPECT_EQ(metadata_buffer[file_frame_index].n_received_packets, 128);
}

TEST(BufferH5Writer, SWMR)
{
    auto root_folder = ".";
    auto device_name = "fast_device";
    size_t pulse_id = 0;

    auto output_buffer = make_unique<char[]>(512 * 1024 * 2);
    auto input_buffer = make_unique<uint16_t[]>(1000*512*1024);
    auto pulse_id_buffer = make_unique<uint64_t[]>(1000);

    auto input_ptr = (uint16_t*)(input_buffer.get());
    auto output_ptr = (uint16_t*)(output_buffer.get());
    auto pulse_id_ptr = (uint64_t*)(pulse_id_buffer.get());

    for (size_t i=0; i<512*1024; i++) {
        uint16_t* image_ptr = (uint16_t*)(output_buffer.get());
        image_ptr[i] = 99;
    }

    BufferH5Writer writer(device_name, root_folder);
    writer.set_pulse_id(0);

    auto filename = BufferUtils::get_filename(
            root_folder, device_name, pulse_id);
    H5::H5File input_file(filename, H5F_ACC_RDONLY |  H5F_ACC_SWMR_READ);
    auto image_dataset = input_file.openDataSet("image");
    auto pulse_id_dataset = input_file.openDataSet("pulse_id");

    image_dataset.read(input_ptr, H5::PredType::NATIVE_UINT16);
    EXPECT_EQ(input_ptr[0], 0);
    EXPECT_EQ(input_ptr[512*1024], 0);

    pulse_id_dataset.read(pulse_id_ptr, H5::PredType::NATIVE_UINT64);
    EXPECT_EQ(pulse_id_ptr[0], 0);
    EXPECT_EQ(pulse_id_ptr[1], 0);

    pulse_id = 0;
    writer.set_pulse_id(pulse_id);
//    writer.write(output_buffer.get());

    image_dataset.read(input_ptr, H5::PredType::NATIVE_UINT16);
    EXPECT_EQ(input_ptr[0], 99);
    EXPECT_EQ(input_ptr[512*1024], 0);

    pulse_id_dataset.read(pulse_id_ptr, H5::PredType::NATIVE_UINT64);
    EXPECT_EQ(pulse_id_ptr[0], 0);
    EXPECT_EQ(pulse_id_ptr[1], 0);

    pulse_id = 1;
    writer.set_pulse_id(pulse_id);
//    writer.write(output_buffer.get());

    image_dataset.read(input_ptr, H5::PredType::NATIVE_UINT16);
    EXPECT_EQ(input_ptr[0], 99);
    EXPECT_EQ(input_ptr[512*1024], 99);

    pulse_id_dataset.read(pulse_id_ptr, H5::PredType::NATIVE_UINT64);
    EXPECT_EQ(pulse_id_ptr[0], 0);
    EXPECT_EQ(pulse_id_ptr[1], 1);
}