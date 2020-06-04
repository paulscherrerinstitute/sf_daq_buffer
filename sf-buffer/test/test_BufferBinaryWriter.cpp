#include "BufferBinaryWriter.hpp"
#include "BufferUtils.hpp"
#include <fcntl.h>
#include "gtest/gtest.h"

TEST(BinaryWriter, basic_interaction)
{
    auto root_folder = ".";
    auto device_name = "test_device";
    uint64_t pulse_id = 5;

    BufferBinaryWriter writer(device_name, root_folder);

    BufferBinaryFormat frame_data;
    frame_data.metadata.pulse_id = 1;
    frame_data.metadata.frame_index = 2;
    frame_data.metadata.daq_rec = 3;
    frame_data.metadata.n_recv_packets = 4;

    writer.write(5, &frame_data);

    auto output_filename =
            BufferUtils::get_filename(root_folder, device_name, pulse_id);

    auto read_fd = open(output_filename.c_str(), O_RDONLY);
    ASSERT_NE(read_fd, -1);

    auto file_frame_index = BufferUtils::get_file_frame_index(pulse_id);

    BufferBinaryFormat read_data;

    ::lseek(read_fd, file_frame_index * sizeof(BufferBinaryFormat), SEEK_SET);
    ::read(read_fd, &read_data, sizeof(BufferBinaryFormat));

    ASSERT_EQ(frame_data.FORMAT_MARKER, read_data.FORMAT_MARKER);
    ASSERT_EQ(frame_data.metadata.pulse_id, read_data.metadata.pulse_id);
    ASSERT_EQ(frame_data.metadata.frame_index, read_data.metadata.frame_index);
    ASSERT_EQ(frame_data.metadata.daq_rec, read_data.metadata.daq_rec);
    ASSERT_EQ(frame_data.metadata.n_recv_packets,
              read_data.metadata.n_recv_packets);
}

TEST(BinaryWriter, test_format_marker)
{
    auto root_folder = ".";
    auto device_name = "test_device";
    uint64_t pulse_id = 5;

    BufferBinaryWriter writer(device_name, root_folder);

    BufferBinaryFormat frame_data;
    frame_data.metadata.pulse_id = 1;
    frame_data.metadata.frame_index = 2;
    frame_data.metadata.daq_rec = 3;
    frame_data.metadata.n_recv_packets = 4;

    writer.write(5, &frame_data);

    auto output_filename =
            BufferUtils::get_filename(root_folder, device_name, pulse_id);

    auto read_fd = open(output_filename.c_str(), O_RDONLY);
    ASSERT_NE(read_fd, -1);

    auto file_frame_index = BufferUtils::get_file_frame_index(pulse_id);

    BufferBinaryFormat read_data;

    // One frame before should be empty.
    lseek(read_fd, (file_frame_index-1) * sizeof(BufferBinaryFormat), SEEK_SET);
    read(read_fd, &read_data, sizeof(BufferBinaryFormat));
    ASSERT_NE(read_data.FORMAT_MARKER, '\xBE');

    // One frame after should be empty as well.
    lseek(read_fd, (file_frame_index+1) * sizeof(BufferBinaryFormat), SEEK_SET);
    read(read_fd, &read_data, sizeof(BufferBinaryFormat));
    ASSERT_NE(read_data.FORMAT_MARKER, '\xBE');

    // But this frame should be here.
    lseek(read_fd, (file_frame_index) * sizeof(BufferBinaryFormat), SEEK_SET);
    read(read_fd, &read_data, sizeof(BufferBinaryFormat));
    ASSERT_EQ(read_data.FORMAT_MARKER, '\xBE');
}