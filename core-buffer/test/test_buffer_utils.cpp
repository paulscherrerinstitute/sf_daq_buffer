#include "gtest/gtest.h"
#include "BufferUtils.hpp"
#include "buffer_config.hpp"

using namespace std;

TEST(BufferUtils, get_filename)
{
    auto expected_file = "/root/device-1/12300000/12345000" +
                         buffer_config::FILE_EXTENSION;

    auto root_folder = "/root";
    auto device_name = "device-1";

    auto result = BufferUtils::get_filename(
            root_folder,
            device_name,
            12345000);

    ASSERT_EQ(result, expected_file);

    auto result2 = BufferUtils::get_filename(
            root_folder,
            device_name,
            12345999);

    ASSERT_EQ(result2, expected_file);

    auto result3 = BufferUtils::get_filename(
            root_folder,
            device_name,
            12346000);

    ASSERT_NE(result3, expected_file);

    auto result4 = BufferUtils::get_filename(
            root_folder,
            device_name,
            12344999);

    ASSERT_NE(result4, expected_file);
}

TEST(BufferUtils, get_file_frame_index)
{
    ASSERT_EQ(BufferUtils::get_file_frame_index(12345000), 0);
    ASSERT_EQ(BufferUtils::get_file_frame_index(12345543), 543);
    ASSERT_EQ(BufferUtils::get_file_frame_index(12345999), 999);
}

TEST(BufferUtils, get_path_suffixes)
{
    auto suffixes = BufferUtils::get_path_suffixes(15100, 18000);
    ASSERT_EQ(suffixes[0].start_pulse_id, 15000);
    ASSERT_EQ(suffixes[0].stop_pulse_id, 15999);
    ASSERT_EQ(suffixes[0].path, "//0/15000.h5");

    ASSERT_EQ(suffixes[1].start_pulse_id, 16000);
    ASSERT_EQ(suffixes[1].stop_pulse_id, 16999);
    ASSERT_EQ(suffixes[1].path, "//0/16000.h5");

    ASSERT_EQ(suffixes[2].start_pulse_id, 17000);
    ASSERT_EQ(suffixes[2].stop_pulse_id, 17999);
    ASSERT_EQ(suffixes[2].path, "//0/17000.h5");

    ASSERT_EQ(suffixes[3].start_pulse_id, 18000);
    ASSERT_EQ(suffixes[3].stop_pulse_id, 18999);
    ASSERT_EQ(suffixes[3].path, "//0/18000.h5");
}