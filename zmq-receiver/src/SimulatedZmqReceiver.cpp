//
//  Weather update client in C++
//  Connects SUB socket to tcp://localhost:5556
//  Collects weather updates and finds avg temp in zipcode
//

#include <zmq.hpp>
#include <iostream>
#include <sstream>

int main (int argc, char *argv[]){
    if (argc != 2) {
        std::cout << "\nERROR\nUsage: jf_buffer_writer [zmq_port]\n";
        exit(-1);
    }

    int zmq_port = atoi(argv[1]);
    std::string addr("tcp://localhost:" + std::to_string(zmq_port));
    
    zmq::context_t context (1);

    //  Socket to talk to server
    std::cout << "Subscribing to server...\n" << std::endl;
    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect(addr.c_str());

    //  Subscribe to IMAGEDATA
	const char *filter = "IMAGEDATA";
    subscriber.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));

    //  Process 100 updates
    int num_img = 0;
    long total_temp = 0;
    zmq::message_t msg_topic;
    zmq::message_t msg_meta;
    zmq::message_t msg_image;
    std::cout << "I'm listening...\n" << std::endl;
    for (int idx = 0; idx < 100000; idx++) {
        subscriber.recv(&msg_topic);
        if(msg_topic.size()==strlen(filter)){
            subscriber.recv(&msg_meta);
            subscriber.recv(&msg_image);
            num_img++;
        }

        if(idx%500==0){ 
            std::cout << "Received " << idx << " (at size " << msg_image.size() << " )\t Received: " << num_img << std::endl;
        }
    }
    return 0;
}
