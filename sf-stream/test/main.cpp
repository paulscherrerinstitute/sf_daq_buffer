#include "gtest/gtest.h"
#include "test_LiveRecvModule.cpp"
#include "test_FastQueue.cpp"

using namespace std;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
