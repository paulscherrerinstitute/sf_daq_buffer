#ifndef SF_DAQ_BUFFER_ZMQPULSERECEIVER_HPP
#define SF_DAQ_BUFFER_ZMQPULSERECEIVER_HPP


#include <cstddef>
#include <string>
#include <vector>

#include "formats.hpp"

class ZmqPulseReceiver {

    const std::vector<std::string> ipc_urls_;
    const int n_modules_;
    void* ctx_;

    std::vector<void*> sockets_;

    void* connect_socket(const std::string url);

public:
    ZmqPulseReceiver(const std::vector<std::string>& ipc_urls, void* ctx);
    ~ZmqPulseReceiver();

    uint64_t get_next_pulse_id();
};


#endif //SF_DAQ_BUFFER_ZMQPULSERECEIVER_HPP
