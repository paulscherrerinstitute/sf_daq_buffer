#include <memory>

#include "ImageAssembler.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace buffer_config;

TEST(ImageAssembler, basic_interaction)
{
    size_t n_modules = 3;
    uint64_t bunch_id = 0;

    ImageAssembler assembler(n_modules);

    ASSERT_EQ(assembler.is_slot_free(bunch_id), true);

    auto buffer_block = make_unique<BufferBinaryBlock>();
    auto buffer_ptr = buffer_block.get();

    for (size_t i_module=0; i_module < n_modules; i_module++) {
        assembler.process(bunch_id, i_module, buffer_ptr);
    }

    ASSERT_EQ(assembler.is_slot_full(bunch_id), true);

    auto metadata = assembler.get_metadata_buffer(bunch_id);
    auto data = assembler.get_data_buffer(bunch_id);

    assembler.free_slot(bunch_id);
    ASSERT_EQ(assembler.is_slot_free(bunch_id), true);

    for (size_t i_pulse = 0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {
        ASSERT_EQ(metadata->is_good_image[i_pulse], 0);
    }
}

TEST(ImageAssembler, reconstruction)
{
    size_t n_modules = 2;
    uint64_t bunch_id = 0;

    ImageAssembler assembler(n_modules);

    ASSERT_EQ(assembler.is_slot_free(bunch_id), true);

    auto buffer_block = make_unique<BufferBinaryBlock>();
    auto buffer_ptr = buffer_block.get();

    for (size_t i_module=0; i_module < n_modules; i_module++) {

        for (size_t i_pulse=0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {
            auto& frame_meta = buffer_block->frame[i_pulse].metadata;

            frame_meta.pulse_id = 100 + i_pulse;
            frame_meta.daq_rec = 1000 + i_pulse;
            frame_meta.frame_index = 10000 + i_pulse;
            frame_meta.n_recv_packets = JF_N_PACKETS_PER_FRAME;

            for (size_t i_pixel=0; i_pixel < MODULE_N_PIXELS; i_pixel++) {
                buffer_block->frame[i_pulse].data[i_pixel] =
                        (i_module * 10) + (i_pixel % 100);
            }
        }

        assembler.process(bunch_id, i_module, buffer_ptr);
    }

    ASSERT_EQ(assembler.is_slot_full(bunch_id), true);

    auto metadata = assembler.get_metadata_buffer(bunch_id);
    auto data = assembler.get_data_buffer(bunch_id);

    assembler.free_slot(bunch_id);
    ASSERT_EQ(assembler.is_slot_free(bunch_id), true);

    ASSERT_EQ(metadata->block_start_pulse_id, 0);
    ASSERT_EQ(metadata->block_stop_pulse_id, BUFFER_BLOCK_SIZE-1);

    for (size_t i_pulse = 0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {
        ASSERT_EQ(metadata->pulse_id[i_pulse], 100 + i_pulse);
        ASSERT_EQ(metadata->daq_rec[i_pulse], 1000 + i_pulse);
        ASSERT_EQ(metadata->frame_index[i_pulse], 10000 + i_pulse);
        ASSERT_EQ(metadata->is_good_image[i_pulse], 1);

        for (size_t i_module=0; i_module < n_modules; i_module++) {
            // TODO: Check assembled image.
        }
    }
}
