#ifndef SF_DAQ_BUFFER_LIVERECVMODULE_HPP
#define SF_DAQ_BUFFER_LIVERECVMODULE_HPP

#include <vector>
#include <thread>

#include "FastQueue.hpp"
#include "jungfrau.hpp"
#include "formats.hpp"
#include "ZmqLiveReceiver.hpp"


class LiveRecvModule {

    FastQueue<ModuleFrameBuffer>& queue_;
    ZmqLiveReceiver& receiver_;

    std::atomic_bool is_receiving_;
    std::thread receiving_thread_;

    void receive_thread();

    void stop();

public:
    LiveRecvModule(
            FastQueue<ModuleFrameBuffer>& queue,
            ZmqLiveReceiver& receiver);

    ~LiveRecvModule();
};


#endif //SF_DAQ_BUFFER_LIVERECVMODULE_HPP
