#include <iostream>
#include <stdexcept>
#include <zmq.h>

#include "../../core-buffer/include/formats.hpp"
#include "../include/JfjFrameCache.hpp"
#include "../include/JfjFrameWorker.hpp"


void dummy_sender(ImageBinaryFormat& image){
    std::cout << "Sending " << image.meta.frame_index << std::endl;
}



int main (int argc, char *argv[]) {
    std::cout << "Creating frame cache..." << std::endl;
    FrameCache cache(32, 3, &dummy_sender);

    std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> push_cb =
        std::bind(&FrameCache::emplace, &cache, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    std::cout << "Creating workers..." << std::endl;
    JfjFrameWorker W0(5005, 1, push_cb);
    //JfjFrameWorker W1(5005, 0, push_cb);
    // JfjFrameWorker W2(5007, 2, push_cb);

    std::thread T0(&JfjFrameWorker::run, &W0);
    //std::thread T1(&JfjFrameWorker::run, &W1);


    T0.join();
    //T1.join();
    std::cout << "Exiting program..." << std::endl;
    return 0;
}
