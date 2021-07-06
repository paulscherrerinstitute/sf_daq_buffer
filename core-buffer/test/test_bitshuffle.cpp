#include <chrono>
#include "gtest/gtest.h"
#include "buffer_config.hpp"

extern "C" {
    #include "bitshuffle/bitshuffle.h"
}

#ifdef USE_EIGER
#include "eiger.hpp"
#else
#include "jungfrau.hpp"
#endif

using namespace std;
using namespace buffer_config;

TEST(bitshuffle, simple_compression)
{
    auto compress_buffer_size = bshuf_compress_lz4_bound(
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);

    auto frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
    auto compress_buffer = make_unique<char[]>(compress_buffer_size);

    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        frame_buffer[i] = i%100;
    }

    auto compressed_size = bshuf_compress_lz4(
            frame_buffer.get(), compress_buffer.get(),
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);

    ASSERT_TRUE(compressed_size > 0);

    auto out_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
    auto consumed_bytes = bshuf_decompress_lz4(
            compress_buffer.get(), out_frame_buffer.get(),
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);

    ASSERT_TRUE(consumed_bytes > 0);

    ASSERT_EQ(compressed_size, consumed_bytes);

    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        ASSERT_EQ(out_frame_buffer[i], frame_buffer[i]);
    }
}

TEST(bitshuffle, separate_compression)
{
    auto compress_buffer_size = bshuf_compress_lz4_bound(
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
    auto frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);

    // Set specific values to pixels and compress each chunk individually.
    auto compress_buffer_1 = make_unique<char[]>(compress_buffer_size);
    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        frame_buffer[i] = i%100;
    }
    auto compressed_size_1 = bshuf_compress_lz4(
            frame_buffer.get(), compress_buffer_1.get(),
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
    ASSERT_TRUE(compressed_size_1 > 0);

    auto compress_buffer_2 = make_unique<char[]>(compress_buffer_size);
    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        frame_buffer[i] = (i%100) + 100;
    }
    auto compressed_size_2 = bshuf_compress_lz4(
            frame_buffer.get(), compress_buffer_2.get(),
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
    ASSERT_TRUE(compressed_size_2 > 0);

    // Allocate common compression buffer to concat them together.
    auto sum_compressed_size = compressed_size_1 + compressed_size_2;
    auto sum_compressed_buffer = make_unique<char[]>(sum_compressed_size);

    // Concat the 2 buffers one after the other.
    memcpy(
            &(sum_compressed_buffer[0]),
            &(compress_buffer_1[0]),
            compressed_size_1);
    memcpy(
            &(sum_compressed_buffer[compressed_size_1]),
            &(compress_buffer_2[0]),
            compressed_size_2);

    // Verify that the memcpy was correct.
    for (size_t i=0; i<compressed_size_1; i++) {
        ASSERT_EQ(sum_compressed_buffer[i], compress_buffer_1[i]);
    }
    auto offset = compressed_size_1;
    for (size_t i=0; i<compressed_size_2; i++) {
        ASSERT_EQ(sum_compressed_buffer[offset+i], compress_buffer_2[i]);
    }

    // Decompress concat buffer to new frame.
    auto out_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS*2);
    auto consumed_bytes = bshuf_decompress_lz4(
            sum_compressed_buffer.get(), out_frame_buffer.get(),
            MODULE_N_PIXELS*2, PIXEL_N_BYTES, MODULE_N_PIXELS);
    ASSERT_EQ(consumed_bytes, sum_compressed_size);

    // Verify that concat output buffer has correct specific values.
    for (size_t i=0;i<MODULE_N_PIXELS*2;i++) {
        if (i<MODULE_N_PIXELS) {
            ASSERT_EQ(out_frame_buffer[i], i%100);
        } else {
            auto i_adj = i - MODULE_N_PIXELS;
            ASSERT_EQ(out_frame_buffer[i], (i_adj%100) + 100);
        }
    }
}

using namespace std;
using namespace chrono;

TEST(bitshuffle, compression_speed)
{
    const int n_iterations = 100;
    const size_t n_pixels = MODULE_N_PIXELS*32;

    auto frame_buffer = make_unique<uint16_t[]>(n_pixels);
    for (size_t i=0; i<n_pixels; i++) {
        frame_buffer[i] = i%100;
    }

    auto compression_buffer = make_unique<char[]>(
            bshuf_compress_lz4_bound(n_pixels, PIXEL_N_BYTES, 0));

    auto start_time = steady_clock::now();

    for (int i=0; i<n_iterations; i++) {
        auto compressed_size = bshuf_compress_lz4(
                frame_buffer.get(), // in
                compression_buffer.get(), // out
                n_pixels,  // size
                PIXEL_N_BYTES,  // elem_size
                0); // block_size
    }

    auto end_time = steady_clock::now();

    size_t duration = duration_cast<microseconds>(
            end_time-start_time).count();

    cout << endl << "Duration of " << duration/100 << " microseconds" << endl;
}