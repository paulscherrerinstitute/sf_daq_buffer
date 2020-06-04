#include "gtest/gtest.h"
#include "test_JFH5Writer.cpp"
#include "test_ImageAssembler.cpp"

using namespace std;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
