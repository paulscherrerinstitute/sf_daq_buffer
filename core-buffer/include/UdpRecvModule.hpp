#ifndef UDPRECVMODULE_HPP
#define UDPRECVMODULE_HPP

#include "RingBuffer.hpp"
#include "FastQueue.hpp"
#include "jungfrau.hpp"
#include <thread>

class UdpRecvModule {

        FastQueue<ModuleFrame>& queue_;
        std::thread receiving_thread_;
        std::atomic_bool is_receiving_;

    protected:
        void receive_thread(const uint16_t udp_port);

    public:
        UdpRecvModule(FastQueue<ModuleFrame>& queue, const uint16_t udp_port);
        virtual ~UdpRecvModule();

};


#endif // UDPRECVMODULE_HPP
