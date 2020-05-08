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
        cout << endl;
    #endif

    receiving_thread_ = thread(
            &LiveRecvModule::receive_thread, this, n_modules);
}

LiveRecvModule::~LiveRecvModule()
{
    is_receiving_ = false;
    receiving_thread_.join();
}

void* LiveRecvModule::connect_socket(size_t module_id)
{
    void* sock = zmq_socket(ctx_, ZMQ_SUB);
    if (sock == nullptr) {
        throw runtime_error(zmq_strerror(errno));
    }

    int rcvhwm = STREAM_RCV_QUEUE_SIZE;
    if (zmq_setsockopt(sock, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }
    int linger = 0;
    if (zmq_setsockopt(sock, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    stringstream ipc_addr;
    ipc_addr << ipc_prefix_ << module_id;
    const auto ipc = ipc_addr.str();

    if (zmq_connect(sock, ipc.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_setsockopt(sock, ZMQ_SUBSCRIBE, "", 0) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    return sock;
}

void LiveRecvModule::recv_single_module(
        void* socket, ModuleFrame* metadata, char* data)
{
    auto n_bytes_metadata = zmq_recv(
            socket,
            metadata,
            sizeof(ModuleFrame),
            0);

    if (n_bytes_metadata != sizeof(ModuleFrame)) {
        throw runtime_error("Stream header of wrong size.");
    }

    if (metadata->pulse_id == 0) {
        throw runtime_error("Received invalid pulse_id=0.");
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

uint64_t LiveRecvModule::align_modules(
        const vector<void*>& sockets, ModuleFrameBuffer *metadata, char *data)
{
    uint64_t max_pulse_id = 0;

    // First pass - determine current max_pulse_id.
    for (size_t i_module = 0; i_module < n_modules_; i_module++) {
        auto& module_metadata = metadata->module[i_module];
        max_pulse_id = max(max_pulse_id, module_metadata.pulse_id);
    }

    // Second pass - align all receivers to max_pulse_id.
    for (size_t i_module = 0; i_module < n_modules_; i_module++) {
        auto& module_metadata = metadata->module[i_module];

        size_t diff_to_max = max_pulse_id - module_metadata.pulse_id;
        for (size_t i = 0; i < diff_to_max; i++) {
            recv_single_module(
                    sockets[i_module],
                    &module_metadata,
                    data + (MODULE_N_BYTES * i_module));
        }

        if (module_metadata.pulse_id != max_pulse_id) {
            throw runtime_error("Cannot align pulse_ids.");
        }
    }

    return max_pulse_id;
}

void LiveRecvModule::receive_thread(const size_t n_modules)
{
    try {

        vector<void*> sockets(n_modules);

        for (size_t i = 0; i < n_modules; i++) {
            sockets[i] = connect_socket(i);
        }

        auto slot_id = queue_.reserve();
        if (slot_id == -1) throw runtime_error("This cannot really happen");

        auto metadata = queue_.get_metadata_buffer(slot_id);
        auto data = queue_.get_data_buffer(slot_id);

        // First buffer load for alignment.
        for (size_t i_module = 0; i_module < n_modules; i_module++) {
            auto& module_metadata = metadata->module[i_module];

            recv_single_module(
                    sockets[i_module],
                    &module_metadata,
                    data + (MODULE_N_BYTES * i_module));
        }

        auto current_pulse_id = align_modules(sockets, metadata, data);

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

            bool sync_needed = false;
            for (size_t i_module = 0; i_module < n_modules; i_module++) {
                auto& module_metadata = metadata->module[i_module];

                recv_single_module(
                        sockets[i_module],
                        &module_metadata,
                        data + (MODULE_N_BYTES * i_module));

                if (module_metadata.pulse_id != current_pulse_id) {
                    sync_needed = true;
                }
            }

            if (sync_needed) {
                auto start_time = chrono::steady_clock::now();

                auto new_pulse_id = align_modules(sockets, metadata, data);
                auto lost_pulses = new_pulse_id - current_pulse_id;
                current_pulse_id = new_pulse_id;

                auto end_time = chrono::steady_clock::now();
                auto us_duration = chrono::duration_cast<chrono::microseconds>(
                        end_time-start_time).count();

                cout << "sf_stream:sync_lost_pulses " << lost_pulses;
                cout << " sf_stream::sync_us " << us_duration;
                cout << endl;
            }

            queue_.commit();
            current_pulse_id++;
        }

        for (size_t i = 0; i < n_modules; i++) {
            zmq_close(sockets[i]);
        }

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