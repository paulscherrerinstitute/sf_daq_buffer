#include <iostream>
#include <thread>
#include <stdexcept>
#include <zmq.h>

#include "BufferUtils.hpp"
#include "formats.hpp"
#include "../include/JfjFrameCache.hpp"
#include "../include/JfjFrameWorker.hpp"
#include "../include/ZmqImagePublisher.hpp"


int main (int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "\nUsage: jf_buffer_writer [detector_json_filename]\n";
        std::cout << "\tdetector_json_filename: detector config file path." << std::endl;
        exit(-1);
    }
    const auto config = BufferUtils::read_json_config(std::string(argv[1]));


    std::cout << "Creating ZMQ socket..." << std::endl;
    ZmqImagePublisher pub("*", 5200, 2);
    // ... and extracting sender function
    std::function<void(ImageBinaryFormat&)> zmq_publish =
        std::bind(&ZmqImagePublisher::sendImage, &pub, std::placeholders::_1);


    std::cout << "Creating frame cache..." << std::endl;
    FrameCache cache(128, 3, zmq_publish);
    // ... and extracting push function
    std::function<void(uint64_t, uint64_t, BufferBinaryFormat&)> push_cb =
        std::bind(&FrameCache::emplace, &cache, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);


    std::cout << "Creating frame workers..." << std::endl;
    std::vector<std::shared_ptr<JfjFrameWorker>> vWorkers;
    for(int mm=0; mm<config.n_modules; mm++){
        // Module name (not really used...)
        char m_name[128];
        snprintf(m_name, 128, "M%02d", mm);
        std::string moduleName(m_name);
        vWorkers.emplace_back( std::make_shared<JfjFrameWorker>(config.start_udp_port+mm, moduleName, mm, push_cb) );
    }


    std::cout << "Starting frame worker threads..." << std::endl;
    std::vector<std::thread> vThreads;
    for(int mm=0; mm<config.n_modules; mm++){
        vThreads.push_back( std::thread(&JfjFrameWorker::run, vWorkers[mm].get()) );
    }

    for(auto& it: vThreads){
        it.join();
    }

    std::cout << "Exiting program..." << std::endl;
    return 0;
}
