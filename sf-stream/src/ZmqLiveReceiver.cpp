#include "ZmqLiveReceiver.hpp"

#include <zmq.h>
#include <stdexcept>
#include <sstream>
#include <chrono>

#include "buffer_config.hpp"
#include "stream_config.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace stream_config;


ZmqLiveReceiver::ZmqLiveReceiver(
        const size_t n_modules,
        void* ctx,
        const std::string &ipc_prefix) :
            n_modules_(n_modules),
            ctx_(ctx),
            ipc_prefix_(ipc_prefix),
            sockets_(n_modules)
{
    for (size_t i = 0; i < n_modules_; i++) {
        sockets_[i] = connect_socket(i);
    }
}

ZmqLiveReceiver::~ZmqLiveReceiver()
{
    for (auto& socket:sockets_) {
        zmq_close(socket);
    }
}

void* ZmqLiveReceiver::connect_socket(size_t module_id)
{
    void* socket = zmq_socket(ctx_, ZMQ_SUB);
    if (socket == nullptr) {
        throw runtime_error(zmq_strerror(errno));
    }

    int rcvhwm = STREAM_RCVHWM;
    if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    int linger = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    stringstream ipc_addr;
    ipc_addr << ipc_prefix_ << module_id;
    const auto ipc = ipc_addr.str();

    if (zmq_connect(socket, ipc.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    return socket;
}

void ZmqLiveReceiver::recv_single_module(
        void* socket, ModuleFrame* meta, char* data)
{
    auto n_bytes_meta = zmq_recv(socket, meta, sizeof(ModuleFrame), 0);

    if (n_bytes_meta == -1) {
        throw runtime_error(zmq_strerror(errno));
    }
    if (n_bytes_meta != sizeof(ModuleFrame)) {
        throw runtime_error("Stream header of wrong size.");
    }
    if (meta->pulse_id == 0) {
        throw runtime_error("Received invalid pulse_id=0.");
    }

    auto n_bytes_frame = zmq_recv(socket, data, MODULE_N_BYTES, 0);

    if (n_bytes_frame == -1) {
        throw runtime_error(zmq_strerror(errno));
    }
    if (n_bytes_frame != MODULE_N_BYTES) {
        throw runtime_error("Stream data of wrong size.");
    }
}

uint64_t ZmqLiveReceiver::align_modules(ModuleFrameBuffer *meta, char *data)
{
    uint64_t max_pulse_id = 0;
    uint64_t min_pulse_id = numeric_limits<uint64_t>::max();

    // First pass - determine current min and max pulse_id.
    for (auto& module_meta : meta->module) {
        min_pulse_id = min(min_pulse_id, module_meta.pulse_id);
        max_pulse_id = max(max_pulse_id, module_meta.pulse_id);
    }

    auto max_diff = max_pulse_id - min_pulse_id;
    if (max_diff > PULSE_OFFSET_LIMIT) {
        stringstream err_msg;

        err_msg << "[ZmqLiveReceiver::align_modules]";
        err_msg << " PULSE_OFFSET_LIMIT exceeded.";
        err_msg << " Modules out of sync for " << max_diff << " pulses.";

        for (auto& module_meta : meta->module) {
            err_msg << " (" << module_meta.module_id << ", ";
            err_msg << module_meta.pulse_id << "),";
        }

        err_msg << endl;

        throw runtime_error(err_msg.str());
    }

    // Second pass - align all receivers to max_pulse_id.
    for (size_t i_module = 0; i_module < n_modules_; i_module++) {
        auto& module_meta = meta->module[i_module];

        while (module_meta.pulse_id < max_pulse_id) {
            recv_single_module(
                    sockets_[i_module],
                    &module_meta,
                    data + (MODULE_N_BYTES * i_module));
        }

        if (module_meta.pulse_id != max_pulse_id) {
            throw runtime_error("Cannot align pulse_ids.");
        }
    }

    return max_pulse_id - min_pulse_id;
}

uint64_t ZmqLiveReceiver::get_next_image(ModuleFrameBuffer* meta, char* data)
{
    uint64_t frame_pulse_id;
    bool sync_needed = false;

    for (size_t i_module = 0; i_module < n_modules_; i_module++) {
        auto& module_meta = meta->module[i_module];

        char* buffer = data + (MODULE_N_BYTES * i_module);
        recv_single_module(sockets_[i_module], &module_meta, buffer);

        if (i_module == 0) {
            frame_pulse_id = module_meta.pulse_id;
        } else if (frame_pulse_id != module_meta.pulse_id) {
            sync_needed = true;
        }
    }

    if (sync_needed) {
        auto lost_pulses = align_modules(meta, data);
        return lost_pulses;
    }

    return 0;
}
