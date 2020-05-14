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

    reader.get_frame(pulse_id, &r_metadata, (char*)&(r_frame_buffer[0]));

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
}
