#include "gtest/gtest.h"
#include "RamBuffer.hpp"

using namespace std;
using namespace buffer_config;


TEST(RamBuffer, simple_store)
{
    const int n_modules = 3;
    const size_t DATA_N_BYTES = MODULE_N_PIXELS * 2;

    RamBuffer buffer("test_detector",
            sizeof(ModuleFrame), DATA_N_BYTES, n_modules, 10);

    ModuleFrame frame_meta;
    frame_meta.id = 12345678;
    frame_meta.pulse_id = 123523;
    frame_meta.daq_rec = 1234;
    frame_meta.frame_index = 12342300;
    frame_meta.n_recv_packets = 128;

    auto frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);


    for (size_t i = 0; i < MODULE_N_PIXELS; i++) {
        frame_buffer[i] = i % 100;
    }

    for (int i_module=0; i_module<n_modules; i_module++) {
        frame_meta.module_id = i_module;

        buffer.write_frame(frame_meta, (char *) (frame_buffer.get()));
    }

    auto meta_buffer = (ModuleFrame*) buffer.get_slot_meta(frame_meta.id);
    for (int i_module=0; i_module<n_modules; i_module++) {
        ASSERT_EQ(meta_buffer[i_module].id, frame_meta.id);
        ASSERT_EQ(meta_buffer[i_module].pulse_id, frame_meta.pulse_id);
        ASSERT_EQ(meta_buffer[i_module].daq_rec, frame_meta.daq_rec);
        ASSERT_EQ(meta_buffer[i_module].frame_index, frame_meta.frame_index);
        ASSERT_EQ(meta_buffer[i_module].module_id, i_module);
    }

    auto data_buffer = (uint16_t*) buffer.get_slot_data(frame_meta.id);
    for (int i_module=0; i_module<n_modules; i_module++) {
        auto module_buffer = data_buffer + (MODULE_N_PIXELS * i_module);

        for (size_t i = 0; i < MODULE_N_PIXELS; i++) {
            ASSERT_EQ(module_buffer[i], i%100);
        }
    }

}
