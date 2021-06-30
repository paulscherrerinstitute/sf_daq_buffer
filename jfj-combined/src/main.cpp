#include <iostream>
#include <thread>
#include <stdexcept>
#include <zmq.h>

#include "../../core-buffer/include/formats.hpp"
#include "../include/JfjFrameCache.hpp"
#include "../include/JfjFrameWorker.hpp"
#include "../include/ZmqImagePublisher.hpp"


int main (int argc, char *argv[]) {
    if (argc != 2) {
        cout << endl;
        cout << "Usage: jf_buffer_writer [detector_json_filename]";
        cout << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = read_json_config(string(argv[1]));

//    // Module name
//    char mn[128];
//    snprintf(mn, 128, "M%02d", module_id);
//    std::string moduleName(mn);



    std::cout << "Creating ZMQ socket..." << std::endl;
//    ZmqImagePublisher pub("129.129.144.76", 5158);
    ZmqImagePublisher pub("*", 5158);
    std::function<void(ImageBinaryFormat&)> zmq_publish =
        std::bind(&ZmqImagePublisher::sendImage, &pub, std::placeholders::_1);

    std::cout << "Creating frame cache..." << std::endl;
    FrameCache cache(32, 3, zmq_publish);

    std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> push_cb =
        std::bind(&FrameCache::emplace, &cache, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    std::cout << "Creating workers..." << std::endl;
    std::vector<JfjFrameWorker> vWorkers;

    JfjFrameWorker W0(5005, "JOCH3M", 0, push_cb);
    JfjFrameWorker W1(5006, "JOCH3M", 1, push_cb);
    JfjFrameWorker W2(5007, "JOCH3M", 2, push_cb);

    std::cout << "Starting worker threads..." << std::endl;
    std::vector<std::thread> vThreads;

    std::thread T0(&JfjFrameWorker::run, &W0);
    std::thread T1(&JfjFrameWorker::run, &W1);
    std::thread T2(&JfjFrameWorker::run, &W2);


    T0.join();
    T1.join();
    T2.join();
    std::cout << "Exiting program..." << std::endl;
    return 0;
}
