#include "gtest/gtest.h"

using namespace core_buffer;

// TODO: Rewrite tests with new format.

//TEST(BufferH5Writer, basic_interaction)
//{
//    auto root_folder = ".";
//    auto device_name = "fast_device";
//    size_t pulse_id = 1;
//
//    auto buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);
//
//    ModuleFrame metadata;
//    metadata.pulse_id = 1;
//    metadata.frame_index = 2;
//    metadata.daq_rec = 3;
//    metadata.n_received_packets = 128;
//
//    BufferH5Writer writer(root_folder, device_name);
//    writer.set_pulse_id(pulse_id);
//    writer.write(&metadata, buffer.get());
//    writer.close_file();
//
//    auto filename = BufferUtils::get_filename(
//            root_folder, device_name, pulse_id);
//
//    auto file_frame_index = BufferUtils::get_file_frame_index(pulse_id);
//
//    H5::H5File input_file(filename, H5F_ACC_RDONLY);
//
//    auto image_dataset = input_file.openDataSet("image");
//    size_t image_buffer_n_bytes = JUNGFRAU_DATA_BYTES_PER_FRAME * FILE_MOD;
//    auto image_buffer = make_unique<uint16_t[]>(image_buffer_n_bytes);
//    image_dataset.read(image_buffer.get(), H5::PredType::NATIVE_UINT16);
//
//    auto metadata_dataset = input_file.openDataSet("metadata");
//    auto metadata_buffer = make_unique<ModuleFrame[]>(FILE_MOD);
//    metadata_dataset.read(metadata_buffer.get(), H5::PredType::NATIVE_UINT64);
//
//    EXPECT_EQ(metadata_buffer[file_frame_index].pulse_id, 1);
//    EXPECT_EQ(metadata_buffer[file_frame_index].frame_index, 2);
//    EXPECT_EQ(metadata_buffer[file_frame_index].daq_rec, 3);
//    EXPECT_EQ(metadata_buffer[file_frame_index].n_received_packets, 128);
//}
//
//TEST(BufferH5Writer, SWMR)
//{
//    auto root_folder = ".";
//    auto device_name = "fast_device";
//    size_t pulse_id = 0;
//
//    auto i_write_buffer = make_unique<char[]>(JUNGFRAU_DATA_BYTES_PER_FRAME);
//    size_t image_buffer_n_bytes = JUNGFRAU_DATA_BYTES_PER_FRAME * FILE_MOD;
//    auto i_read_buffer = make_unique<uint16_t[]>(image_buffer_n_bytes);
//
//    ModuleFrame m_write_buffer = {1, 2, 3, 4, 5};
//    auto m_read_buffer = make_unique<ModuleFrame[]>(FILE_MOD);
//
//    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
//        uint16_t* image_ptr = (uint16_t*)(i_write_buffer.get());
//        image_ptr[i] = 99;
//    }
//
//    BufferH5Writer writer(device_name, root_folder);
//    // This creates the file.
//    writer.set_pulse_id(0);
//
//    auto filename = BufferUtils::get_filename(
//            root_folder, device_name, pulse_id);
//
//    H5::H5File input_file(filename, H5F_ACC_RDONLY |  H5F_ACC_SWMR_READ);
//    auto image_dataset = input_file.openDataSet("image");
//    auto metadata_dataset = input_file.openDataSet("metadata");
//
//    // The data was not yet written to file, so 0 is expected.
//    image_dataset.read(i_read_buffer.get(), H5::PredType::NATIVE_UINT16);
//    EXPECT_EQ((i_read_buffer.get())[0], 0);
//    EXPECT_EQ((i_read_buffer.get())[512 * 1024], 0);
//
//    // The data was not yet written to file, so 0 is expected.
//    metadata_dataset.read(m_read_buffer.get(), H5::PredType::NATIVE_UINT64);
//    EXPECT_EQ((m_read_buffer.get())[0].pulse_id, 0);
//    EXPECT_EQ((m_read_buffer.get())[1].pulse_id, 0);
//
//    // Flushing after every frame should ensure that the reader can see this.
//    writer.set_pulse_id(0);
//    writer.write(&m_write_buffer, i_write_buffer.get());
//
//    image_dataset.read(i_read_buffer.get(), H5::PredType::NATIVE_UINT16);
//    // Frame 0 was written, so we are expecting data in just the first frame.
//    EXPECT_EQ((i_read_buffer.get())[0], 99);
//    EXPECT_EQ((i_read_buffer.get())[512 * 1024], 0);
//
//    // Frame 0 written, metadata for frame 0 expected.
//    metadata_dataset.read(m_read_buffer.get(), H5::PredType::NATIVE_UINT64);
//    EXPECT_EQ((m_read_buffer.get())[0].pulse_id, 1);
//    EXPECT_EQ((m_read_buffer.get())[0].frame_index, 2);
//    EXPECT_EQ((m_read_buffer.get())[0].daq_rec, 3);
//    EXPECT_EQ((m_read_buffer.get())[0].n_received_packets, 4);
//    EXPECT_EQ((m_read_buffer.get())[0].module_id, 5);
//
//    EXPECT_EQ((m_read_buffer.get())[1].pulse_id, 0);
//    EXPECT_EQ((m_read_buffer.get())[1].frame_index, 0);
//    EXPECT_EQ((m_read_buffer.get())[1].daq_rec, 0);
//    EXPECT_EQ((m_read_buffer.get())[1].n_received_packets, 0);
//    EXPECT_EQ((m_read_buffer.get())[1].module_id, 0);
//
//    writer.set_pulse_id(1);
//    writer.write(&m_write_buffer, i_write_buffer.get());
//
//    image_dataset.read(i_read_buffer.get(), H5::PredType::NATIVE_UINT16);
//    // Both frame written, and we should access both.
//    EXPECT_EQ((i_read_buffer.get())[0], 99);
//    EXPECT_EQ((i_read_buffer.get())[512 * 1024], 99);
//
//    // Both frame written, and we should access both.
//    metadata_dataset.read(m_read_buffer.get(), H5::PredType::NATIVE_UINT64);
//    EXPECT_EQ((m_read_buffer.get())[0].pulse_id, 1);
//    EXPECT_EQ((m_read_buffer.get())[0].frame_index, 2);
//    EXPECT_EQ((m_read_buffer.get())[0].daq_rec, 3);
//    EXPECT_EQ((m_read_buffer.get())[0].n_received_packets, 4);
//    EXPECT_EQ((m_read_buffer.get())[0].module_id, 5);
//
//    EXPECT_EQ((m_read_buffer.get())[1].pulse_id, 1);
//    EXPECT_EQ((m_read_buffer.get())[1].frame_index, 2);
//    EXPECT_EQ((m_read_buffer.get())[1].daq_rec, 3);
//    EXPECT_EQ((m_read_buffer.get())[1].n_received_packets, 4);
//    EXPECT_EQ((m_read_buffer.get())[1].module_id, 5);
//}