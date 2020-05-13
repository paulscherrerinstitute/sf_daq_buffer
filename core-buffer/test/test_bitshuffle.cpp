#include "gtest/gtest.h"
#include "buffer_config.hpp"
#include "bitshuffle/bitshuffle.h"

using namespace std;

TEST(bitshuffle, simple_compression)
{
    auto size = MODULE_N_PIXELS;
    auto elem_size = 2; // uint16_t
    auto block_size = MODULE_N_BYTES;

    auto compress_buffer_size = bshuf_compress_lz4_bound(
            size, elem_size, block_size);

    auto frame_buffer = make_unique<char[]>(MODULE_N_BYTES);
    auto compress_buffer = make_unique<char[]>(compress_buffer_size);

    auto compressed_size = bshuf_compress_lz4(
            frame_buffer.get(), compress_buffer.get(),
            size, elem_size, block_size);

    ASSERT_TRUE(compressed_size > 0);

    auto out_frame_buffer = make_unique<char[]>(MODULE_N_BYTES);
    auto consumed_bytes = bshuf_decompress_lz4(
            compress_buffer.get(), out_frame_buffer.get(),
            size, elem_size, block_size);

    ASSERT_TRUE(consumed_bytes > 0);

    ASSERT_EQ(compressed_size, consumed_bytes);
}