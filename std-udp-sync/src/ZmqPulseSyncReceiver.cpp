#include "ZmqPulseSyncReceiver.hpp"
#include "BufferUtils.hpp"

#include <zmq.h>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <iostream>
#include "date.h"

#include "sync_config.hpp"

using namespace std;
using namespace chrono;
using namespace sync_config;


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
    uint64_t ids[n_modules_];

    for (uint32_t i_sync=0; i_sync < SYNC_RETRY_LIMIT; i_sync++) {
        bool modules_in_sync = true;
        for (int i = 0; i < n_modules_; i++) {

            zmq_recv(sockets_[i], &ids[i], sizeof(uint64_t), 0);

            if (ids[0] != ids[i]) {
                modules_in_sync = false;
            }
        }

        if (modules_in_sync) {
            #ifdef DEBUG_OUTPUT
                using namespace date;
                cout << "[" << std::chrono::system_clock::now() << "]";
                cout << " [ZmqPulseSyncReceiver::get_next_pulse_id]";
                cout << " Modules in sync (";
                cout << " pulse_id " << ids[0] <<").";
                cout << endl;
            #endif
            return {ids[0], i_sync};
        }
        
        #ifdef DEBUG_OUTPUT
            using namespace date;
            cout << "[" << std::chrono::system_clock::now() << "]";
            cout << " [ZmqPulseSyncReceiver::get_next_pulse_id]";
            cout << " Modules out of sync:" << endl;
            for (int i=0; i < n_modules_; i++) {
                cout << " module" << i << ":" << ids[i];
            }
            cout << endl;
        #endif
    }

    stringstream err_msg;
    err_msg << "[ZmqPulseSyncReceiver::get_next_pulse_id]";
    err_msg << " SYNC_RETRY_LIMIT exceeded. State:";
    for (int i=0; i < n_modules_; i++) {
        err_msg << " module" << i << ":" << ids[i];
    }
    err_msg << endl;

    throw runtime_error(err_msg.str());
}
