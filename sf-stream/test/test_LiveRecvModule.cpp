#include <zmq.h>
#include "LiveRecvModule.hpp"
#include "gtest/gtest.h"
#include "buffer_config.hpp"
#include <future>

using namespace std;
using namespace buffer_config;

TEST(LiveRecvModule, transfer_test) {
    // TODO: Make this test work again.
//    auto ctx = zmq_ctx_new();
//
//    size_t n_modules = 32;
//    size_t n_slots = 5;
//    FastQueue<ModuleFrameBuffer> queue(MODULE_N_BYTES * n_modules, n_slots);
//
//    void *sockets[n_modules];
//    for (size_t i = 0; i < n_modules; i++) {
//        sockets[i] = zmq_socket(ctx, ZMQ_PUB);
//
//        int linger = 0;
//        if (zmq_setsockopt(sockets[i], ZMQ_LINGER, &linger,
//                           sizeof(linger)) != 0) {
//            throw runtime_error(zmq_strerror(errno));
//        }
//
//        stringstream ipc_addr;
//        ipc_addr << BUFFER_LIVE_IPC_URL << i;
//        const auto ipc = ipc_addr.str();
//
//        if (zmq_bind(sockets[i], ipc.c_str()) != 0) {
//            throw runtime_error(zmq_strerror(errno));
//        }
//    }
//
//    LiveRecvModule recv_module(queue, n_modules, ctx, BUFFER_LIVE_IPC_URL);
//
//    // Nothing should be committed, queue, should be empty.
//    ASSERT_EQ(queue.read(), -1);
//
//    ModuleFrame metadata;
//    auto data = make_unique<char[]>(MODULE_N_BYTES);
//
//    for (size_t i = 0; i < n_modules; i++) {
//        metadata.pulse_id = 1;
//        metadata.frame_index = 2;
//        metadata.daq_rec = 3;
//        metadata.n_received_packets = 4;
//        metadata.module_id = i;
//
//        zmq_send(sockets[i], &metadata, sizeof(ModuleFrame), ZMQ_SNDMORE);
//        zmq_send(sockets[i], data.get(), MODULE_N_BYTES, 0);
//    }
//
//    this_thread::sleep_for(chrono::milliseconds(100));
//
//    auto slot_id = queue.read();
//    // We should have the first Detector frame in the buffer.
//    //ASSERT_NE(slot_id, -1);
//
//    auto recv_stopped = async(launch::async, [&](){
//        recv_module.stop();
//    });
//
//    this_thread::sleep_for(chrono::milliseconds(100));
//
//    for (size_t i = 0; i < n_modules; i++) {
//        metadata.pulse_id = 1;
//        metadata.frame_index = 2;
//        metadata.daq_rec = 3;
//        metadata.n_received_packets = 4;
//        metadata.module_id = i;
//
//        zmq_send(sockets[i], &metadata, sizeof(ModuleFrame), ZMQ_SNDMORE);
//        zmq_send(sockets[i], data.get(), MODULE_N_BYTES, 0);
//    }
//
//    recv_stopped.wait();
//
//    for (size_t i = 0; i < n_modules; i++) {
//        zmq_close(sockets[i]);
//    }
//
//    zmq_ctx_destroy(ctx);
//    cout << "We are finished" << endl;
}