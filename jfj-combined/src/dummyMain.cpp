#include <iostream>
#include <stdexcept>
#include <zmq.h>

#include "formats.hpp"
#include "../include/JfjFrameCache.hpp"
#include "../include/JfjFrameWorker.hpp"


void dummy_sender(ImageMetadata* meta, std::vector<char>* data){
    std::cout << "Sending " << meta->frame_index << std::endl;
}



int main (int argc, char *argv[]) {





    FrameCache cache(32, 3, JFJOCH_DATA_BYTES_PER_MODULE, &dummy_sender);
    JfjFrameWorker W0(5005, 0, cache.emplace);
    JfjFrameWorker W1(5006, 1, cache.emplace);
    JfjFrameWorker W2(5007, 2, cache.emplace);

}
