#include "gtest/gtest.h"
#include "test_PacketUdpReceiver.cpp"
#include "test_FrameUdpReceiver.cpp"

using namespace std;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
