#include <gtest/gtest.h>
#include <thread>

#include "ReplayH5Reader.hpp"
#include "BufferH5Writer.hpp"

using namespace std;
using namespace core_buffer;

TEST(ReplayH5Reader, basic_interaction)
{
    auto root_folder = ".";
    auto device_name = "fast_device";
    size_t pulse_id = 65;
    uint16_t source_id = 124;

    // This 2 must be compatible by design.
    BufferH5Writer writer(root_folder, device_name);
    ReplayH5Reader reader(root_folder, device_name);

    ModuleFrame w_metadata;
    ModuleFrame* r_metadata;
    auto w_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
    char* r_frame_buffer;

    // Setup test values.
    w_metadata.pulse_id = pulse_id;
    w_metadata.frame_index = 2;
    w_metadata.daq_rec = 3;
    w_metadata.n_received_packets = 128;
    w_metadata.module_id = source_id;

    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        w_frame_buffer[i] = i % 100;
    }

    // Write to file.
    writer.set_pulse_id(pulse_id);
    writer.write(&w_metadata, (char*)&(w_frame_buffer[0]));
    writer.close_file();

    reader.get_buffer(pulse_id, r_metadata, r_frame_buffer);

    ASSERT_EQ(r_metadata->pulse_id, pulse_id);
    ASSERT_EQ(r_metadata->module_id, source_id);
    ASSERT_EQ(r_metadata->frame_index, 2);
    ASSERT_EQ(r_metadata->daq_rec, 3);
    ASSERT_EQ(r_metadata->n_received_packets, 128);

    // Data as well.
    auto offset = MODULE_N_PIXELS * (pulse_id-1);
    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        w_frame_buffer[i] = r_frame_buffer[offset + i];
    }

    for (uint64_t i_pulse=0; i_pulse<100; i_pulse++) {

        // Verify that all but the saved pulse_id are zero.
        if (i_pulse == pulse_id) {
            continue;
        }

        reader.get_buffer(i_pulse, r_metadata, r_frame_buffer);

        ASSERT_EQ(r_metadata->pulse_id, 0);
        ASSERT_EQ(r_metadata->frame_index, 0);
        ASSERT_EQ(r_metadata->daq_rec, 0);
        ASSERT_EQ(r_metadata->n_received_packets, 0);
        ASSERT_EQ(r_metadata->module_id, 0);
    }

    reader.close_file();
}
