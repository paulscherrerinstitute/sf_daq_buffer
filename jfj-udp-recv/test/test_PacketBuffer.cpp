#include <netinet/in.h>
#include <jungfraujoch.hpp>
#include "gtest/gtest.h"
#include "PacketBuffer.hpp"

#include <thread>
#include <chrono>
#include <future>

using namespace std;



std::ostream &operator<<(std::ostream &os, jfjoch_packet_t const &packet) { 
    os << "Frame number: " << packet.framenum << std::endl;
    os << "Packet number: " << packet.packetnum << std::endl;
    os << "Bunch id: " << packet.bunchid << std::endl;
    os << std::endl;
    return os;
}




class MockReceiver{
    public:
        uint64_t idx_packet = 42000;
        uint64_t packet_per_frame = 512;
        uint64_t num_bunches = 100;
        uint64_t num_packets =50;

        uint64_t receive_many(mmsghdr* msgs, const size_t n_msgs){
            // Receive 'num_packets numner of packets'
            for(int ii=0; ii<num_packets; ii++){
                std::cout << "Buffer length: " << msgs[ii].msg_hdr.msg_iov->iov_len << "\tExpected: " << sizeof(jfjoch_packet_t)  << std::endl;
                
                jfjoch_packet_t& refer = reinterpret_cast<jfjoch_packet_t&>(msgs[ii].msg_hdr.msg_iov->iov_base);
                refer.framenum = idx_packet / packet_per_frame;
                refer.bunchid = idx_packet / packet_per_frame;
                refer.packetnum = idx_packet % packet_per_frame;
                
                std::cout << refer << "\n";
                idx_packet++;
            }
            return num_packets;
        };
};



TEST(BufferUdpReceiver, packetbuffer_simple){
    std::cout << "Testing PacketBuffer..." << std::endl;

    PacketBuffer<jfjoch_packet_t, 128> p_buffer;
    MockReceiver mockery;
    uint64_t prev_bunch, prev_packet;
    jfjoch_packet_t p_pop;
    
    mockery.idx_packet = 7*512 + 13;
    mockery.num_packets = 25;
    
    p_buffer.fill_from(mockery);
    
    // First packet
    ASSERT_FALSE(p_buffer.is_empty());
    ASSERT_EQ(p_buffer.size(), 25);

    std::cout << "Current packet:\n" << p_buffer.peek_front();
    
    
    ASSERT_EQ(p_buffer.peek_front().bunchid, 7);
    ASSERT_EQ(p_buffer.peek_front().packetnum, 13);
    prev_bunch = p_buffer.peek_front().bunchid;
    prev_packet = p_buffer.peek_front().packetnum;
    
    p_pop = p_buffer.pop_front();
    ASSERT_EQ(p_buffer.size(), 24);

    ASSERT_EQ(p_pop.bunchid, prev_bunch);
    ASSERT_EQ(p_pop.packetnum, prev_packet);
    ASSERT_EQ(p_buffer.peek_front().bunchid, prev_bunch);
    ASSERT_EQ(p_buffer.peek_front().packetnum, prev_packet+1);
    std::cout << "Done..." << std::endl;

};
