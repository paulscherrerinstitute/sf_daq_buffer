#include "gtest/gtest.h"
#include "test_UdpReceiver.cpp"
#include "test_UdpRecvModule.cpp"
#include "test_BinaryWriter.cpp"
#include "test_buffer_utils.cpp"
#include "test_BufferH5Writer.cpp"
#include "test_SFWriter.cpp"
#include "test_FastQueue.cpp"

using namespace std;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
