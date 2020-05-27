#ifndef SF_DAQ_BUFFER_BLOCKZMQSENDER_HPP
#define SF_DAQ_BUFFER_BLOCKZMQSENDER_HPP

#include <string>
#include <jungfrau.hpp>
#include <formats.hpp>

class BlockZmqSender {

    void* ctx_;
    void* socket_;

public:
    BlockZmqSender(const std::string& ipc_id, const int source_id);
    virtual ~BlockZmqSender();

    void close();

    void send(const BufferBinaryBlock* block_data);
};


#endif //SF_DAQ_BUFFER_BLOCKZMQSENDER_HPP
