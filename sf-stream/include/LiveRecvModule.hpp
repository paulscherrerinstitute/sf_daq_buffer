#ifndef SF_DAQ_BUFFER_LIVERECVMODULE_HPP
#define SF_DAQ_BUFFER_LIVERECVMODULE_HPP

#include <vector>
#include <thread>

#include "FastQueue.hpp"
#include "jungfrau.hpp"
#include "formats.hpp"

class LiveRecvModule {

    FastQueue<ModuleFrameBuffer>& queue_;
    const size_t n_modules_;
    void* ctx_;
    const std::string ipc_prefix_;
    std::atomic_bool is_receiving_;
    std::thread receiving_thread_;

    void* connect_socket(size_t module_id);
    void receive_thread();
    void recv_single_module(void* socket, ModuleFrame* meta, char* data);
    uint64_t align_modules(const std::vector<void*>& sockets,
                           ModuleFrameBuffer *metadata,
                           char *data);
    void stop();

public:
    LiveRecvModule(
            FastQueue<ModuleFrameBuffer>& queue,
            const size_t n_modules,
            void* ctx,
            const std::string& ipc_prefix);

    ~LiveRecvModule();
};


#endif //SF_DAQ_BUFFER_LIVERECVMODULE_HPP
