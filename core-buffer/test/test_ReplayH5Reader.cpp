#include "ReplayH5Reader.hpp"
#include "BufferH5Writer.hpp"
#include "gtest/gtest.h"

using namespace core_buffer;

TEST(ReplayH5Reader, basic_interaction)
{
    auto root_folder = ".";
    auto device_name = "fast_device";

    // This 2 must be compatible by design.
    BufferH5Writer writer(root_folder, device_name);
    ReplayH5Reader reader(root_folder, device_name);

    size_t pulse_id = 65;

    ModuleFrame w_metadata;
    ModuleFrame r_metadata;
    auto w_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
    auto r_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);

    // Setup test values.
    w_metadata.pulse_id = pulse_id;
    w_metadata.frame_index = 2;
    w_metadata.daq_rec = 3;
    w_metadata.n_received_packets = 128;
    w_metadata.module_id = 4;

    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        w_frame_buffer[i] = i % 100;
    }

    // Write to file.
    writer.set_pulse_id(pulse_id);
    writer.write(&w_metadata, (char*)&(w_frame_buffer[0]));
    writer.close_file();

    auto frame_present = reader.get_frame(
            pulse_id, &r_metadata, (char*)&(r_frame_buffer[0]));

    ASSERT_EQ(frame_present, true);

    // Metadata has to match.
    EXPECT_EQ(r_metadata.pulse_id, w_metadata.pulse_id);
    EXPECT_EQ(r_metadata.frame_index, w_metadata.frame_index);
    EXPECT_EQ(r_metadata.daq_rec, w_metadata.daq_rec);
    EXPECT_EQ(r_metadata.n_received_packets, w_metadata.n_received_packets);
    EXPECT_EQ(r_metadata.module_id, w_metadata.module_id);

    // Data as well.
    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        w_frame_buffer[i] = r_frame_buffer[i];
    }

    reader.close_file();
}

TEST(ReplayH5Reader, missing_frame)
{
    auto root_folder = ".";
    auto device_name = "fast_device";

    // This 2 must be compatible by design.
    BufferH5Writer writer(root_folder, device_name);
    ReplayH5Reader reader(root_folder, device_name);

    size_t pulse_id = 65;

    ModuleFrame w_metadata;
    ModuleFrame r_metadata;
    auto w_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
    auto r_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);

    // Setup test values.
    w_metadata.pulse_id = pulse_id;
    w_metadata.frame_index = 2;
    w_metadata.daq_rec = 3;
    w_metadata.n_received_packets = 128;
    w_metadata.module_id = 4;

    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        w_frame_buffer[i] = i % 100;
    }

    // Write to file.
    writer.set_pulse_id(pulse_id);
    writer.write(&w_metadata, (char*)&(w_frame_buffer[0]));
    writer.close_file();

    // But read another pulse_id, that should be empty.
    auto frame_present = reader.get_frame(
            pulse_id-1, &r_metadata, (char*)&(r_frame_buffer[0]));

    ASSERT_EQ(frame_present, false);

    // All metadata has to be 0, expect pulse_id.
    EXPECT_EQ(r_metadata.pulse_id, pulse_id-1);
    EXPECT_EQ(r_metadata.frame_index, 0);
    EXPECT_EQ(r_metadata.daq_rec, 0);
    EXPECT_EQ(r_metadata.n_received_packets, 0);
    EXPECT_EQ(r_metadata.module_id, 0);

    // Data as well.
    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        r_frame_buffer[i] = 0;
    }

    reader.close_file();
}
