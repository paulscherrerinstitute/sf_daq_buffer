#ifndef SF_DAQ_BUFFER_REPLAYZMQSENDER_HPP
#define SF_DAQ_BUFFER_REPLAYZMQSENDER_HPP

#include <string>
#include <jungfrau.hpp>
#include <formats.hpp>

class ReplayZmqSender {

    void* ctx_;
    void* socket_;

public:
    ReplayZmqSender(const std::string& ipc_id, const int source_id);
    virtual ~ReplayZmqSender();

    void close();

    void send(const BufferBinaryBlock* metadata, const char* data);
};


#endif //SF_DAQ_BUFFER_REPLAYZMQSENDER_HPP
