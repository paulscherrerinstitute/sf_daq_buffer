#include <netinet/in.h>
#include <jungfraujoch.hpp>
#include "gtest/gtest.h"
#include "PacketBuffer.hpp"

#include <thread>
#include <chrono>
#include <future>

using namespace std;

class MockReceiver{
    public:
        int idx_packet = 42000;
        int packet_per_frame = 512;
        int num_bunches = 100;
        int num_packets =50;

        int receive_many(mmsghdr* msgs, const size_t n_msgs){
            // Receive 'num_packets numner of packets'
            
            for(int ii=0; ii<std::min(size_t(num_packets), n_msgs); ii++){
                jfjoch_packet_t& refer = reinterpret_cast<jfjoch_packet_t&>(msgs[ii].msg_hdr.msg_iov->iov_base);
                refer.bunchid = idx_packet / packet_per_frame;
                refer.packetnum = idx_packet % packet_per_frame;
                idx_packet++;
            }
            return std::min(size_t(num_packets), n_msgs);
        };
};





TEST(BufferUdpReceiver, packetbuffer_simple){
    std::cout << "Testing PacketBuffer..." << std::endl;

    
    PacketBuffer<jfjoch_packet_t, 128> p_buffer;
    MockReceiver mockery;
    uint64_t prev_bunch, prev_packet;
    jfjoch_packet_t p_pop;
    
    mockery.num_packets = 25;
    mockery.idx_packet = 42000;
    
    p_buffer.fill_from(mockery);
    
    // First packet
    ASSERT_EQ(p_buffer.peek_front().bunchid, 42000/512+1);
    prev_bunch = p_buffer.peek_front().bunchid;
    prev_packet = p_buffer.peek_front().packetnum;
    
    ASSERT_EQ(p_buffer.size(), 25);
    p_pop = p_buffer.pop_front();
    ASSERT_EQ(p_buffer.size(), 24);

    ASSERT_EQ(p_pop.bunchid, prev_bunch);
    ASSERT_EQ(p_pop.packetnum, prev_packet);
    ASSERT_EQ(p_buffer.peek_front().bunchid, prev_bunch);
    ASSERT_EQ(p_buffer.peek_front().packetnum, prev_packet+1);
    std::cout << "Done..." << std::endl;

};
