#ifndef SF_DAQ_BUFFER_LIVERECVMODULE_HPP
#define SF_DAQ_BUFFER_LIVERECVMODULE_HPP

#include "FastQueue.hpp"
#include <thread>
#include "jungfrau.hpp"

class LiveRecvModule {

    FastQueue<ModuleFrameBuffer>& queue_;
    const size_t n_modules_;
    void* ctx_;
    const std::string ipc_prefix_;
    std::atomic_bool is_receiving_;
    std::thread receiving_thread_;

public:
    LiveRecvModule(
            FastQueue<ModuleFrameBuffer>& queue,
            const size_t n_modules,
            void* ctx,
            const std::string& ipc_prefix);

    virtual ~LiveRecvModule();
    void* connect_socket(size_t module_id);
    void recv_single_module(void* socket, char* metadata, char* data);
    void receive_thread(const size_t n_modules, void* ctx);
};


#endif //SF_DAQ_BUFFER_LIVERECVMODULE_HPP
