#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include <chrono>

#include "../../core-buffer/include/buffer_config.hpp"
#include "../include/BufferTypes.hpp"





int main (int argc, char *argv[]){
    if (argc != 4) {
        std::cout << "\nERROR\nUsage: jf_buffer_writer [zmq_topic] [zmq_sub_addr] [detector_name]\n";
        std::cout << "Topic corresponds to a supported format: IMAGEDATA\n";
        exit(-1);
    }


    std::string topic(argv[1]);
    std::string sub_raw(argv[2]);
    std::string detector_name(argv[3]);
    std::string sub_addr("tcp://" + sub_raw);

    std::cout << "Starting ZMQ receiver at:\nPORT:\t" << sub_addr << "\nTOPIC:\t" << topic << std::endl;

    // Allocate the RamBuffer
    //RamBuffer buffer(config.detector_name, config.n_modules);

    //  ZMQ communication setup
    std::cout << "Subscribing to server...\n" << std::endl;
    zmq::context_t context (1);
    //  Subscribe to TOPIC (expected schema)
    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect(sub_addr.c_str());
    //subscriber.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    // Publisher to ipc
    std::cout << "Crating publisher...\n" << std::endl;

    zmq::socket_t republisher (context, ZMQ_PUB);
//    const int sndhwm = buffer_config::BUFFER_ZMQ_SNDHWM;
//    republisher.setsockopt(ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
//    const int linger = 0;
//    republisher.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));

//    std::string ipc_addr = buffer_config::BUFFER_LIVE_IPC_URL + detector_name + "-assembler";
    std::string ipc_addr = "tcp://*:5333";
    try{
        republisher.bind(ipc_addr.c_str());
    } catch (std::exception& ex){
        std::string msg = "Failed to bing publisher to address '" + ipc_addr + "': " + ex.what();
        throw std::runtime_error(msg);

    }


    //  Process 100 updates
    int num_img = 0;
    long total_temp = 0;
    zmq::message_t msg_topic;
    zmq::message_t msg_meta;
    zmq::message_t msg_data;


    ImageMetadataCache cache("./dataout", "raspicam");

    std::cout << "I'm listening...\n" << std::endl;
    for (int idx = 0; idx < 100000; idx++) {
        // ZMQ guarantees full delivery of multipart massages!
        // Packets are sent as three part messages:  topic + meta + data
        //subscriber.recv(&msg_topic, 0);
        subscriber.recv(&msg_meta, 0);
        subscriber.recv(&msg_data, 0);

        // Schema (topic) specific saving)
        if(topic=="IMAGEDATA"){
            auto t1 = std::chrono::high_resolution_clock::now();
            cache.append((void*)msg_meta.data(), msg_meta.size(), (void*)msg_data.data(), msg_data.size());
            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
            
            std::cout << "Append took: " << ms_double.count() << " ms" << std::endl;
            //buffer.write_image((ImageMetadata*)msg_meta.data(), (char*)msg_data.data);
            if(idx%100==0){
                std::cout << "Received " << idx << " (at size " << msg_data.size() << " )" << std::endl;
            }
        }
    }
    return 0;
}
















