#include <BinaryReader.hpp>
#include "gtest/gtest.h"

TEST(BinaryReader, basic_interaction) {
    // TODO: Write some real tests.
    auto detector_folder = "test_device";
    auto module_name = "M1";
    BinaryReader reader(detector_folder, module_name);
}

