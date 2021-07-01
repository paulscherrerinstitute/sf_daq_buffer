#include <thread>
#include <chrono>
#include <cstring>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include "../../core-buffer/include/jungfrau.hpp"


void MockDetector(uint16_t udp_port, int32_t moduleId, int32_t sleep_ms){
    auto send_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    if(send_socket_fd < 0){std::cout << "Failed to create socket" << std::endl; exit(-1); };

    struct sockaddr_in server_address, client_address;
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    // Filling server information
    server_address.sin_family    = AF_INET; // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(udp_port);
    
    // Send loop
    jungfrau_packet send_udp_buffer;
    memset(&send_udp_buffer, 0, sizeof(send_udp_buffer));

    for(int64_t ff=0; ff<10000; ff++){
        send_udp_buffer.framenum = ff;
        send_udp_buffer.bunchid = ff;
        send_udp_buffer.moduleID = moduleId;
        send_udp_buffer.debug = 0;        
        
    	for(int64_t pp=0; pp<128; pp++){
            send_udp_buffer.packetnum = pp;
            ::sendto(send_socket_fd, &send_udp_buffer, sizeof(send_udp_buffer), 0, (sockaddr*) &server_address, sizeof(server_address));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        if(ff%1000==0){
            std::cout << "Sent " << ff << " frames" << std::endl;
            }
    }

    //close(send_socket_fd);
}


int main (int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << "\nERROR\nUsage: jf_buffer_writer [num_modules] [start_port] [sleep_ms]\n";
        exit(-1);
    }

    int num_modules = atoi(argv[1]);
    int start_port = atoi(argv[2]);
    int sleep_ms = atoi(argv[3]);
    sleep_ms = (sleep_ms>=1) ? sleep_ms : 1;
    

    std::cout << "Starting worker threads..." << std::endl;
    std::vector<std::thread> vThreads;
    
    for(int mm=0; mm<num_modules; mm++){
        vThreads.push_back( std::thread(&MockDetector, start_port+mm, mm, sleep_ms) );
    }
    std::cout << "Threads are up and running..." << std::endl;


    for(auto& it: vThreads){
        it.join();
    }
    std::cout << "Exiting program..." << std::endl;
    return 0;
}
