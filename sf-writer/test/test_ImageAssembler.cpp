#include <memory>

#include "ImageAssembler.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace core_buffer;

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
}
