#include "ReplayZmqSender.hpp"

#include <sstream>
#include <zmq.h>

#include "buffer_config.hpp"

using namespace std;
using namespace core_buffer;


ReplayZmqSender::ReplayZmqSender(const string& ipc_id, const int source_id)
{
    auto ipc_base = REPLAY_STREAM_IPC_URL + ipc_id + "-";
    stringstream ipc_stream;
    ipc_stream << ipc_base << source_id;
    const auto ipc_address = ipc_stream.str();

    ctx_ = zmq_ctx_new();
    socket_ = zmq_socket(ctx_, ZMQ_PUSH);

    const int sndhwm = REPLAY_SNDHWM;
    if (zmq_setsockopt(socket_, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(zmq_strerror (errno));

    const int linger_ms = -1;
    if (zmq_setsockopt(socket_, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(zmq_strerror (errno));

    if (zmq_bind(socket_, ipc_address.c_str()) != 0)
        throw runtime_error(zmq_strerror (errno));
}

ReplayZmqSender::~ReplayZmqSender()
{
    close();
}

void ReplayZmqSender::close() {
    zmq_close(socket_);
    zmq_ctx_destroy(ctx_);
}

void ReplayZmqSender::send(const ReplayBuffer* metadata, const char* data)
{
    zmq_send(socket_, metadata, sizeof(ReplayBuffer), ZMQ_SNDMORE);
    zmq_send(socket_, data, MODULE_N_BYTES * BUFFER_BLOCK_SIZE, 0);
}