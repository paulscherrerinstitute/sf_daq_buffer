#include "gtest/gtest.h"
#include "BufferUtils.hpp"
#include "buffer_config.hpp"

using namespace std;

TEST(BufferUtils, get_filename)
{
    auto expected_file = "/root/device-1/12300000/12345000" +
                         buffer_config::FILE_EXTENSION;

    auto detector_folder = "/root";
    auto module_name = "device-1";

    auto result = BufferUtils::get_filename(
            detector_folder,
            module_name,
            12345000);

    ASSERT_EQ(result, expected_file);

    auto result2 = BufferUtils::get_filename(
            detector_folder,
            module_name,
            12345999);

    ASSERT_EQ(result2, expected_file);

    auto result3 = BufferUtils::get_filename(
            detector_folder,
            module_name,
            12346000);

    ASSERT_NE(result3, expected_file);

    auto result4 = BufferUtils::get_filename(
            detector_folder,
            module_name,
            12344999);

    ASSERT_NE(result4, expected_file);
}

TEST(BufferUtils, get_file_frame_index)
{
    ASSERT_EQ(BufferUtils::get_file_frame_index(12345000), 0);
    ASSERT_EQ(BufferUtils::get_file_frame_index(12345543), 543);
    ASSERT_EQ(BufferUtils::get_file_frame_index(12345999), 999);
}
