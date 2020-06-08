#include "LiveRecvModule.hpp"
#include "date.h"
#include <iostream>
#include <cstring>
#include "zmq.h"
#include "buffer_config.hpp"
#include "stream_config.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace stream_config;

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
    receiving_thread_ = thread(&LiveRecvModule::receive_thread, this);
}

LiveRecvModule::~LiveRecvModule()
{
    stop();
}

void LiveRecvModule::stop()
{
    is_receiving_ = false;
    receiving_thread_.join();
}

void* LiveRecvModule::connect_socket(size_t module_id)
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

void LiveRecvModule::recv_single_module(
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

uint64_t LiveRecvModule::align_modules(
        const vector<void*>& sockets, ModuleFrameBuffer *meta, char *data)
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

        err_msg << "[LiveRecvModule::align_modules]";
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

        size_t diff_to_max = max_pulse_id - module_meta.pulse_id;
        for (size_t i = 0; i < diff_to_max; i++) {
            recv_single_module(
                    sockets[i_module],
                    &module_meta,
                    data + (MODULE_N_BYTES * i_module));
        }

        if (module_meta.pulse_id != max_pulse_id) {
            throw runtime_error("Cannot align pulse_ids.");
        }
    }

    return max_pulse_id - min_pulse_id;
}

void LiveRecvModule::receive_thread()
{
    try {

        vector<void*> sockets(n_modules_);

        for (size_t i = 0; i < n_modules_; i++) {
            sockets[i] = connect_socket(i);
        }

        int slot_id;
        while(is_receiving_.load(memory_order_relaxed)) {

            while ((slot_id == queue_.reserve()) == -1) {
                this_thread::sleep_for(milliseconds(RB_READ_RETRY_INTERVAL_MS));
            }

            auto metadata = queue_.get_metadata_buffer(slot_id);
            auto data = queue_.get_data_buffer(slot_id);

            uint64_t frame_pulse_id;
            bool sync_needed = false;
            for (size_t i_module = 0; i_module < n_modules_; i_module++) {
                auto& module_metadata = metadata->module[i_module];

                recv_single_module(
                        sockets[i_module],
                        &module_metadata,
                        data + (MODULE_N_BYTES * i_module));

                if (i_module == 0) {
                    frame_pulse_id = module_metadata.pulse_id;
                } else if (frame_pulse_id != module_metadata.pulse_id) {
                    sync_needed = true;
                }
            }

            if (sync_needed) {
                auto start_time = steady_clock::now();

                auto lost_pulses = align_modules(sockets, metadata, data);

                auto end_time = steady_clock::now();
                auto us_duration = duration_cast<microseconds>(
                        end_time-start_time).count();

                cout << "sf_stream:sync_lost_pulses " << lost_pulses;
                cout << " sf_stream::sync_us " << us_duration;
                cout << endl;
            }

            queue_.commit();
        }

        for (size_t i = 0; i < n_modules_; i++) {
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