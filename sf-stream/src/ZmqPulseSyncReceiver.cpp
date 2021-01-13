#include "ZmqPulseSyncReceiver.hpp"
#include "BufferUtils.hpp"

#include <zmq.h>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <iostream>

#include "stream_config.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace stream_config;


ZmqPulseSyncReceiver::ZmqPulseSyncReceiver(
        void * ctx,
        const string& detector_name,
        const int n_modules) :
            ctx_(ctx),
            n_modules_(n_modules)
{
    sockets_.reserve(n_modules_);

    for (int i=0; i<n_modules_; i++) {
        sockets_.push_back(
                BufferUtils::connect_socket(ctx_, detector_name, i));
    }
}

ZmqPulseSyncReceiver::~ZmqPulseSyncReceiver()
{
    for (auto& socket:sockets_) {
        zmq_close(socket);
    }
}

uint64_t ZmqPulseSyncReceiver::get_next_pulse_id() const
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

    for (int i_sync=0; i_sync < SYNC_RETRY_LIMIT; i_sync++) {
        cout << "Sync attempt " << i_sync << endl;

        uint64_t min_pulse_id = 0;
        uint64_t max_pulse_id = numeric_limits<uint64_t>::max();

        for (int i = 0; i < n_modules_; i++) {
            min_pulse_id = min(min_pulse_id, pulses[i]);
            max_pulse_id = max(max_pulse_id, pulses[i]);
        }

        auto max_diff = max_pulse_id - min_pulse_id;
        if (max_diff > PULSE_OFFSET_LIMIT) {
            stringstream err_msg;
            err_msg << "[ZmqPulseSyncReceiver::get_next_pulse_id]";
            err_msg << " PULSE_OFFSET_LIMIT exceeded.";
            err_msg << " max_diff=" << max_diff << " pulses.";

            for (int i = 0; i < n_modules_; i++) {
                err_msg << " (module " << i << ", ";
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
