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
                BufferUtils::connect_socket(ctx_, detector_name, to_string(i)));
    }
}

ZmqPulseSyncReceiver::~ZmqPulseSyncReceiver()
{
    for (auto& socket:sockets_) {
        zmq_close(socket);
    }
}

PulseAndSync ZmqPulseSyncReceiver::get_next_pulse_id() const
{
    uint64_t pulses[n_modules_];

    #ifdef DEBUG_OUTPUT
        cout << "[ZmqPulseSyncReceiver::get_next_pulse_id()]";
        cout << "n_modules_" << n_modules_;
        cout << endl;
    #endif

    bool modules_in_sync = true;
    for (int i = 0; i < n_modules_; i++) {
        zmq_recv(sockets_[i], &pulses[i], sizeof(uint64_t), 0);

        if (pulses[0] != pulses[i]) {
            modules_in_sync = false;
        }
    }

    if (modules_in_sync) {
        return {pulses[0], 0};
    }

    // How many pulses we lost in total to get the next pulse_id.
    uint32_t n_lost_pulses = 0;
    for (int i_sync=0; i_sync < SYNC_RETRY_LIMIT; i_sync++) {
        uint64_t min_pulse_id = numeric_limits<uint64_t>::max();;
        uint64_t max_pulse_id = 0;

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
        // Max pulses we lost in this sync attempt.
        uint32_t i_sync_lost_pulses = 0;
        for (int i = 0; i < n_modules_; i++) {
            // How many pulses we lost for this specific module.
            uint32_t i_module_lost_pulses = 0;
            while (pulses[i] < max_pulse_id) {
                zmq_recv(sockets_[i], &pulses[i], sizeof(uint64_t), 0);
                i_module_lost_pulses++;
            }

            i_sync_lost_pulses = max(i_sync_lost_pulses, i_module_lost_pulses);

            if (pulses[i] != max_pulse_id) {
                modules_in_sync = false;
            }
        }
        n_lost_pulses += i_sync_lost_pulses;

        if (modules_in_sync) {
            return {pulses[0], n_lost_pulses};
        }
    }

    stringstream err_msg;
    err_msg << "[ZmqLiveReceiver::get_next_pulse_id]";
    err_msg << " SYNC_RETRY_LIMIT exceeded.";
    err_msg << endl;

    throw runtime_error(err_msg.str());
}
