
#include "WriterH5Writer.hpp"
#include "gtest/gtest.h"
#include "bitshuffle/bitshuffle.h"


using namespace core_buffer;

TEST(WriterH5Writer, basic_interaction)
{
    size_t n_modules = 2;
    size_t n_frames = 5;

    auto data = make_unique<char[]>(n_modules*MODULE_N_BYTES);
    auto metadata = make_shared<ImageMetadata>();

    // Needed by writer.
    metadata->data_n_bytes = 500;

    WriterH5Writer writer("ignore.h5", n_frames, n_modules, 1);
    writer.write(metadata.get(), data.get());
    writer.close_file();
}

TEST(WriterH5Writer, test_compression)
{
//    size_t n_modules = 2;
//    size_t n_frames = 2;
//
//    auto comp_buffer_size = bshuf_compress_lz4_bound(
//            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
//
//    auto f_raw_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
//    auto f_comp_buffer = make_unique<char[]>(comp_buffer_size);
//
//    auto i_comp_buffer = make_unique<char[]>(
//            (comp_buffer_size * n_modules) + BSHUF_LZ4_HEADER_BYTES);
//    auto i_raw_buffer = make_unique<uint16_t[]>(
//            MODULE_N_PIXELS * n_modules * n_frames);
//
//    bshuf_write_uint64_BE(&i_comp_buffer[0],
//            MODULE_N_BYTES * n_modules);
//    bshuf_write_uint32_BE(&i_comp_buffer[8],
//            MODULE_N_PIXELS * PIXEL_N_BYTES);
//
//    size_t total_compressed_size = BSHUF_LZ4_HEADER_BYTES;
//    for (int i_module=0; i_module<n_modules; i_module++) {
//
//        for (size_t i=0; i<MODULE_N_PIXELS; i++) {
//            f_raw_buffer[i] = (uint16_t)((i % 100) + (i_module*100));
//        }
//
//        auto compressed_size = bshuf_compress_lz4(
//                f_raw_buffer.get(), f_comp_buffer.get(),
//                MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
//
//        memcpy((i_comp_buffer.get() + total_compressed_size),
//               f_comp_buffer.get(),
//               compressed_size);
//
//        total_compressed_size += compressed_size;
//    }
//
//    auto metadata = make_shared<ImageMetadata>();
//    metadata->data_n_bytes = total_compressed_size;
//
//    metadata->is_good_frame = 1;
//    metadata->frame_index = 3;
//    metadata->pulse_id = 3;
//    metadata->daq_rec = 3;
//
//    auto result = bshuf_decompress_lz4(
//            &i_comp_buffer[12], &i_raw_buffer[0],
//            MODULE_N_PIXELS*n_modules, PIXEL_N_BYTES, MODULE_N_PIXELS);
//
//    WriterH5Writer writer("ignore.h5", n_frames, n_modules);
//    writer.write(metadata.get(), &i_comp_buffer[0]);
//    writer.close_file();
//
//    H5::H5File reader("ignore.h5", H5F_ACC_RDONLY);
//    auto image_dataset = reader.openDataSet("image");
//    image_dataset.read(&i_raw_buffer[0], H5::PredType::NATIVE_UINT16);
//
//    for (int i_module=0; i_module<n_modules; i_module++) {
//        for (int i_pixel=0; i_pixel<MODULE_N_PIXELS; i_pixel++) {
//            size_t offset = (i_module * MODULE_N_PIXELS) + i_pixel;
//            ASSERT_EQ(i_raw_buffer[offset],
//                    (uint16_t)((i_pixel % 100) + (i_module*100)));
//        }
//    }
}