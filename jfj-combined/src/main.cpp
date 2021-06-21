#include <iostream>
#include <stdexcept>
#include <zmq.h>

#include "../../core-buffer/include/formats.hpp"
#include "../include/JfjFrameCache.hpp"
#include "../include/JfjFrameWorker.hpp"


void dummy_sender(ImageMetadata* meta, std::vector<char>* data){
    std::cout << "Sending " << meta->frame_index << std::endl;
}



int main (int argc, char *argv[]) {
    std::cout << "Creating frame cache..." << std::endl;
    FrameCache cache(32, 3, JFJOCH_DATA_BYTES_PER_MODULE, &dummy_sender);

    std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> push_cb =
        std::bind(&FrameCache::emplace, &cache, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    std::cout << "Creating workers..." << std::endl;
    JfjFrameWorker W0(5005, 0, push_cb);
    JfjFrameWorker W1(5006, 1, push_cb);
    JfjFrameWorker W2(5007, 2, push_cb);

    std::thread T0(&JfjFrameWorker::run, &W0);


    T0.join();
    std::cout << "Exiting program..." << std::endl;
    return 0;
}
