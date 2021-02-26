#include "gtest/gtest.h"
#include "test_buffer_utils.cpp"
#include "test_bitshuffle.cpp"
#include "test_RamBuffer.cpp"

using namespace std;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
