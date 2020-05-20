#include "ReplayH5Reader.hpp"
#include "BufferH5Writer.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace core_buffer;

TEST(ReplayH5Reader, basic_interaction)
{
    auto root_folder = ".";
    auto device_name = "fast_device";
    size_t pulse_id = 65;
    uint16_t source_id = 1;

    // This 2 must be compatible by design.
    BufferH5Writer writer(root_folder, device_name);
    ReplayH5Reader reader(root_folder, device_name, source_id, pulse_id);

    ModuleFrame w_metadata;
    ReplayModuleFrameBuffer r_metadata;
    auto w_frame_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
    auto r_frame_buffer = make_unique<uint16_t[]>(
            MODULE_N_PIXELS * REPLAY_READ_BUFFER_SIZE);

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

    reader.get_buffer(1, &r_metadata, (char *) &(r_frame_buffer[0]));

    ASSERT_EQ(r_metadata.n_frames, 65);
    ASSERT_EQ(r_metadata.module_id, source_id);
    ASSERT_EQ(r_metadata.data_n_bytes, MODULE_N_BYTES * 65);

    for (int i=0; i<pulse_id-1; i++) {
        ASSERT_EQ(r_metadata.pulse_id[i], i+1);
        ASSERT_EQ(r_metadata.is_frame_present[i], false);
        ASSERT_EQ(r_metadata.is_good_frame[i], false);
    }

    // -1 due to 0 based indexing.
    ASSERT_EQ(r_metadata.pulse_id[pulse_id-1], pulse_id);
    ASSERT_EQ(r_metadata.is_frame_present[pulse_id-1], true);
    ASSERT_EQ(r_metadata.is_good_frame[pulse_id-1], true);


    // Data as well.
    auto offset = MODULE_N_PIXELS * (pulse_id-1);
    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
        w_frame_buffer[i] = r_frame_buffer[offset + i];
    }

    reader.close_file();
}

TEST(ReplayH5Reader, missing_frame)
{
    auto root_folder = ".";
    auto device_name = "fast_device";

    size_t pulse_id = 65;

    // This 2 must be compatible by design.
    BufferH5Writer writer(root_folder, device_name);
    ReplayH5Reader reader(root_folder, device_name, 1, pulse_id);



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

    // TODO: Write test for missing frame as well.

    // But read another pulse_id, that should be empty.
//    auto frame_present = reader.get_buffer(
//            pulse_id - 1, &r_metadata, (char *) &(r_frame_buffer[0]));
//
//    ASSERT_EQ(frame_present, false);
//
//    // All metadata has to be 0, expect pulse_id.
//    EXPECT_EQ(r_metadata.pulse_id, pulse_id-1);
//    EXPECT_EQ(r_metadata.frame_index, 0);
//    EXPECT_EQ(r_metadata.daq_rec, 0);
//    EXPECT_EQ(r_metadata.n_received_packets, 0);
//    EXPECT_EQ(r_metadata.module_id, 0);
//
//    // Data as well.
//    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
//        r_frame_buffer[i] = 0;
//    }

    reader.close_file();
}
