#include "BufferH5Writer.hpp"
#include "gtest/gtest.h"

using namespace core_buffer;

TEST(FastH5Writer, basic_interaction)
{
    auto root_folder = ".";
    auto device_name = "fast_device";
    size_t pulse_id = 1;

    auto buffer = make_unique<char[]>(512*1024*2);

    UdpFrameMetadata metadata;
    metadata.pulse_id = 1;
    metadata.frame_index = 2;
    metadata.daq_rec = 3;
    metadata.n_recv_packets = 128;

    BufferH5Writer writer(
            core_buffer::FILE_MOD, 512, 1024, device_name, root_folder);

    writer.add_scalar_metadata<uint64_t>("pulse_id");
    writer.add_scalar_metadata<uint64_t>("frame_id");
    writer.add_scalar_metadata<uint32_t>("daq_rec");
    writer.add_scalar_metadata<uint16_t>("received_packets");

    writer.set_pulse_id(pulse_id);

    writer.write_data(buffer.get());

    writer.write_scalar_metadata<uint64_t>(
        "pulse_id", &(metadata.pulse_id));

    writer.write_scalar_metadata<uint64_t>(
        "frame_id", &(metadata.frame_index));

    writer.write_scalar_metadata<uint32_t>(
        "daq_rec", &(metadata.daq_rec));

    writer.write_scalar_metadata<uint16_t>(
        "received_packets", &(metadata.n_recv_packets));

    writer.close_file();

    auto filename = BufferUtils::get_filename(
            root_folder, device_name, pulse_id);

    auto file_frame_index = BufferUtils::get_file_frame_index(pulse_id);

    H5::H5File input_file(filename, H5F_ACC_RDONLY);

    auto image_dataset = input_file.openDataSet("image");
    auto image_buffer = make_unique<uint16_t[]>(1000*512*1024);
    image_dataset.read(
            image_buffer.get(), H5::PredType::NATIVE_UINT16);

    auto pulse_id_dataset = input_file.openDataSet("pulse_id");
    auto pulse_id_buffer = make_unique<uint64_t[]>(1000);
    pulse_id_dataset.read(
            pulse_id_buffer.get(), H5::PredType::NATIVE_UINT64);
    EXPECT_EQ(pulse_id_buffer[file_frame_index], 1);

    auto frame_id_dataset = input_file.openDataSet("frame_id");
    auto frame_id_buffer = make_unique<uint64_t[]>(1000);
    frame_id_dataset.read(
            frame_id_buffer.get(), H5::PredType::NATIVE_UINT64);
    EXPECT_EQ(frame_id_buffer[file_frame_index], 2);

    auto daq_rec_dataset = input_file.openDataSet("daq_rec");
    auto daq_rec_buffer = make_unique<uint32_t[]>(1000);
    daq_rec_dataset.read(
            daq_rec_buffer.get(), H5::PredType::NATIVE_UINT32);
    EXPECT_EQ(daq_rec_buffer[file_frame_index], 3);

    auto received_packets_dataset =
            input_file.openDataSet("received_packets");
    auto received_packets_buffer = make_unique<uint16_t[]>(1000);
    received_packets_dataset.read(
            received_packets_buffer.get(), H5::PredType::NATIVE_UINT16);
    EXPECT_EQ(received_packets_buffer[file_frame_index], 128);
}

TEST(FastH5Writer, SWMR)
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

    BufferH5Writer writer(
            FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE, device_name, root_folder);
    writer.add_scalar_metadata<uint64_t>("pulse_id");
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
    writer.write_data(output_buffer.get());
    writer.write_scalar_metadata<uint64_t>("pulse_id", &pulse_id);

    image_dataset.read(input_ptr, H5::PredType::NATIVE_UINT16);
    EXPECT_EQ(input_ptr[0], 99);
    EXPECT_EQ(input_ptr[512*1024], 0);

    pulse_id_dataset.read(pulse_id_ptr, H5::PredType::NATIVE_UINT64);
    EXPECT_EQ(pulse_id_ptr[0], 0);
    EXPECT_EQ(pulse_id_ptr[1], 0);

    pulse_id = 1;
    writer.set_pulse_id(pulse_id);
    writer.write_data(output_buffer.get());
    writer.write_scalar_metadata<uint64_t>("pulse_id", &pulse_id);

    image_dataset.read(input_ptr, H5::PredType::NATIVE_UINT16);
    EXPECT_EQ(input_ptr[0], 99);
    EXPECT_EQ(input_ptr[512*1024], 99);

    pulse_id_dataset.read(pulse_id_ptr, H5::PredType::NATIVE_UINT64);
    EXPECT_EQ(pulse_id_ptr[0], 0);
    EXPECT_EQ(pulse_id_ptr[1], 1);
}