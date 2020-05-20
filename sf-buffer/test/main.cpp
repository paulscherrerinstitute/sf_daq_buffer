#include "gtest/gtest.h"
#include "test_UdpReceiver.cpp"
#include "test_BufferBinaryWriter.cpp"
#include "test_BufferH5Writer.cpp"
#include "test_BufferUdpReceiver.cpp"

using namespace std;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
