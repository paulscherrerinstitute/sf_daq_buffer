#include "LiveRecvModule.hpp"
#include "date.h"
#include <iostream>
#include <cstring>
#include "zmq.h"
#include "buffer_config.hpp"

using namespace std;
using namespace core_buffer;

LiveRecvModule::LiveRecvModule(
        FastQueue<ModuleFrameBuffer>& queue_,
        const size_t n_modules,
        void* ctx_,
        const string& ipc_prefix) :
        queue_(queue_),
        n_modules_(n_modules),
        ctx_(ctx_),
        ipc_prefix_(ipc_prefix),
        is_receiving_(true)
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[LiveRecvModule::LiveRecvModule]";
    #endif

    receiving_thread_ = thread(
            &LiveRecvModule::receive_thread, this,
            n_modules,
            ctx_);
}

LiveRecvModule::~LiveRecvModule()
{
    is_receiving_ = false;
    receiving_thread_.join();
}

void* LiveRecvModule::connect_socket(size_t module_id)
{
    void* sock = zmq_socket(ctx_, ZMQ_SUB);

    int rcvhwm = STREAM_RCV_QUEUE_SIZE;
    if (zmq_setsockopt(sock, ZMQ_RCVHWM, &rcvhwm,
                       sizeof(rcvhwm)) != 0) {
        throw runtime_error(strerror(errno));
    }
    int linger = 0;
    if (zmq_setsockopt(sock, ZMQ_LINGER, &linger,
                       sizeof(linger)) != 0) {
        throw runtime_error(strerror(errno));
    }

    stringstream ipc_addr;
    ipc_addr << ipc_prefix_ << module_id;
    const auto ipc = ipc_addr.str();

    if (zmq_connect(sock, ipc.c_str()) != 0) {
        throw runtime_error(strerror(errno));
    }

    if (zmq_setsockopt(sock, ZMQ_SUBSCRIBE, "",
                       sizeof("")) != 0) {
        throw runtime_error(strerror(errno));
    }

    return sock;
}

void LiveRecvModule::recv_single_module(
        void* socket, char* metadata, char* data)
{
    auto n_bytes_metadata = zmq_recv(
            socket,
            metadata,
            sizeof(ModuleFrame),
            0);

    if (n_bytes_metadata != sizeof(ModuleFrame)) {
        throw runtime_error("Stream header of wrong size.");
    }

    auto n_bytes_image = zmq_recv(
            socket,
            data,
            MODULE_N_BYTES,
            0);

    if (n_bytes_image != MODULE_N_BYTES) {
        throw runtime_error("Stream data of wrong size.");
    }
}

void LiveRecvModule::receive_thread(const size_t n_modules, void* ctx_)
{
    try {

        void *sockets[n_modules];
        for (size_t i = 0; i < n_modules; i++) {
            sockets[i] = connect_socket(i);
        }

        uint64_t current_pulse_id = 0;
        auto slot_id = queue_.read();
        if (slot_id == -1) throw runtime_error("This cannot really happen");

        auto metadata = queue_.get_metadata_buffer(slot_id);
        auto data = queue_.get_data_buffer(slot_id);
        // First pass - determine current max pulse id.
        for (size_t i_module = 0; i_module < n_modules; i_module++) {
            auto module_metadata = metadata->module[i_module];

            recv_single_module(
                    sockets[i_module],
                    (char*)(&(module_metadata)),
                    data + (MODULE_N_BYTES * i_module));

            current_pulse_id = max(
                    current_pulse_id,
                    metadata->module[i_module].pulse_id);
        }

        // Second pass - align all receivers to the max pulse_id.
        for (size_t i_module = 0; i_module < n_modules; i_module++) {
            auto module_metadata = metadata->module[i_module];

            size_t diff_to_max = current_pulse_id - module_metadata.pulse_id;
            for (size_t i = 0; i < diff_to_max; i++) {
                recv_single_module(
                        sockets[i_module],
                        (char *) (&(module_metadata)),
                        data + (MODULE_N_BYTES * i_module));
            }

            if (module_metadata.pulse_id != current_pulse_id) {
                throw runtime_error("Cannot align pulse_ids.");
            }
        }

        queue_.commit();
        current_pulse_id++;

        while(is_receiving_.load(memory_order_relaxed)) {
            auto slot_id = queue_.reserve();

            if (slot_id == -1){
                this_thread::sleep_for(chrono::milliseconds(5));
                continue;
            }

            metadata = queue_.get_metadata_buffer(slot_id);
            data = queue_.get_data_buffer(slot_id);

            for (size_t i_module = 0; i_module < n_modules; i_module++) {
                auto module_metadata = metadata->module[i_module];

                recv_single_module(
                        sockets[i_module],
                        (char*)(&(module_metadata)),
                        data + (MODULE_N_BYTES * i_module));

                if (current_pulse_id != module_metadata.pulse_id) {
                    throw runtime_error("Modules out of sync.");
                }
            }

            queue_.commit();
            current_pulse_id++;
        }

        for (size_t i = 0; i < n_modules; i++) {
            zmq_close(sockets[i]);
        }

        zmq_ctx_destroy(ctx_);

    } catch (const std::exception& e) {
        is_receiving_ = false;

        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[LiveRecvModule::receive_thread]";
        cout << " Stopped because of exception: " << endl;
        cout << e.what() << endl;

        throw;
    }
}