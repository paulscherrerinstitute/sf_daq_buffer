#ifndef SF_DAQ_BUFFER_ZMQLIVERECEIVER_HPP
#define SF_DAQ_BUFFER_ZMQLIVERECEIVER_HPP


#include <cstddef>
#include <string>
#include <vector>

#include "formats.hpp"

class ZmqLiveReceiver {

    const size_t n_modules_;
    void* ctx_;
    const std::string ipc_prefix_;
    std::vector<void*> sockets_;

    void* connect_socket(size_t module_id);
    void recv_single_module(void* socket, ModuleFrame* meta, char* data);
    uint64_t align_modules(ModuleFrameBuffer *meta, char *data);

public:
    ZmqLiveReceiver(const size_t n_modules,
                    void* ctx,
                    const std::string& ipc_prefix);

    ~ZmqLiveReceiver();

    uint64_t get_next_image(ModuleFrameBuffer* meta, char* data);
};


#endif //SF_DAQ_BUFFER_ZMQLIVERECEIVER_HPP
