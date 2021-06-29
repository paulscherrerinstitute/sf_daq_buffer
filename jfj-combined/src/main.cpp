#include <iostream>
#include <stdexcept>
#include <zmq.h>

#include "../../core-buffer/include/formats.hpp"
#include "../include/JfjFrameCache.hpp"
#include "../include/JfjFrameWorker.hpp"
#include "../include/ZmqImagePublisher.hpp"


int main (int argc, char *argv[]) {
    
    std::cout << "Creating ZMQ socket..." << std::endl;
    ZmqImagePublisher pub("129.129.144.76", 5158);
    std::function<void(ImageBinaryFormat&)> zmq_publish =
        std::bind(&ZmqImagePublisher::sendImage, &pub, std::placeholders::_1);

    std::cout << "Creating frame cache..." << std::endl;
    FrameCache cache(32, 3, zmq_publish);

    std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> push_cb =
        std::bind(&FrameCache::emplace, &cache, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    std::cout << "Creating workers..." << std::endl;
    JfjFrameWorker W0(5005, 0, push_cb);
    JfjFrameWorker W1(5006, 1, push_cb);
    JfjFrameWorker W2(5007, 2, push_cb);
    
    std::cout << "Starting worker threads..." << std::endl;
    std::thread T0(&JfjFrameWorker::run, &W0);
    std::thread T1(&JfjFrameWorker::run, &W1);
    std::thread T2(&JfjFrameWorker::run, &W2);


    T0.join();
    T1.join();
    T2.join();
    std::cout << "Exiting program..." << std::endl;
    return 0;
}
