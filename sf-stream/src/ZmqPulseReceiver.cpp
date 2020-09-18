#include "ZmqPulseReceiver.hpp"

#include <zmq.h>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "stream_config.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace stream_config;


ZmqPulseReceiver::ZmqPulseReceiver(
        const vector<string>& ipc_urls,
        void* ctx) :
            ipc_urls_(ipc_urls),
            n_modules_(ipc_urls_.size()),
            ctx_(ctx)
{
    sockets_.reserve(ipc_urls_.size());

    for (const auto& url : ipc_urls_) {
        sockets_.push_back(connect_socket(url));
    }
}

ZmqPulseReceiver::~ZmqPulseReceiver()
{
    for (auto& socket:sockets_) {
        zmq_close(socket);
    }
}

void* ZmqPulseReceiver::connect_socket(const string url)
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

    if (zmq_connect(socket, url.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    return socket;
}

uint64_t ZmqPulseReceiver::get_next_pulse_id()
{
    uint64_t pulses[n_modules_];

    bool modules_in_sync = true;
    for (int i = 0; i < n_modules_; i++) {
        zmq_recv(sockets_[i], &pulses[i], sizeof(uint64_t), 0);

        if (pulses[0] != pulses[i]) {
            modules_in_sync = false;
        }
    }

    if (modules_in_sync) {
        return pulses[0];
    }

    for (int i_sync; i_sync < SYNC_RETRY_LIMIT; i_sync++) {
        uint64_t min_pulse_id = 0;
        uint64_t max_pulse_id = numeric_limits<uint64_t>::max();

        for (int i = 0; i < n_modules_; i++) {
            min_pulse_id = min(min_pulse_id, pulses[i]);
            max_pulse_id = max(max_pulse_id, pulses[i]);
        }

        auto max_diff = max_pulse_id - min_pulse_id;
        if (max_diff > PULSE_OFFSET_LIMIT) {
            stringstream err_msg;
            err_msg << "[ZmqLiveReceiver::get_next_pulse_id]";
            err_msg << " PULSE_OFFSET_LIMIT exceeded.";
            err_msg << " max_diff=" << max_diff << " pulses.";

            for (int i = 0; i < n_modules_; i++) {
                err_msg << " (" << ipc_urls_[i] << ", ";
                err_msg << pulses[i] << "),";
            }
            err_msg << endl;

            throw runtime_error(err_msg.str());
        }

        modules_in_sync = true;
        for (int i = 0; i < n_modules_; i++) {
            while (pulses[i] < max_pulse_id) {
                zmq_recv(sockets_[i], &pulses[i], sizeof(uint64_t), 0);
            }

            if (pulses[i] != max_pulse_id) {
                modules_in_sync = false;
            }
        }

        if (modules_in_sync) {
            return pulses[0];
        }
    }

    stringstream err_msg;
    err_msg << "[ZmqLiveReceiver::get_next_pulse_id]";
    err_msg << " SYNC_RETRY_LIMIT exceeded.";
    err_msg << endl;

    throw runtime_error(err_msg.str());
}
