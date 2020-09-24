#ifndef SF_DAQ_BUFFER_ZMQPULSESYNCRECEIVER_HPP
#define SF_DAQ_BUFFER_ZMQPULSESYNCRECEIVER_HPP


#include <cstddef>
#include <string>
#include <vector>

#include "formats.hpp"

class ZmqPulseSyncReceiver {

    void* ctx_;
    const int n_modules_;

    std::vector<void*> sockets_;

public:
    ZmqPulseSyncReceiver(
            void* ctx,
            const std::string& detector_name,
            const int n_modules);
    ~ZmqPulseSyncReceiver();

    uint64_t get_next_pulse_id() const;
};


#endif //SF_DAQ_BUFFER_ZMQPULSESYNCRECEIVER_HPP
