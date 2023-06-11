#ifndef SF_DAQ_BUFFER_ZMQ_IMAGE_PUBLISHER_HPP
#define SF_DAQ_BUFFER_ZMQ_IMAGE_PUBLISHER_HPP

#include <iostream>
#include <zmq.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "../../core-buffer/include/formats.hpp"


#define ASSERT_FALSE(expr, msg)                                                                                         \
    if(bool(expr)) {                                                                                                     \
        std::string text = "ASSERTION called at " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + "\n"; \
        text = text + "Reason: " + std::to_string(expr) + "\n";                                                         \
        text = text + "Message:" + msg + "\nErrno: " + std::to_string(errno);                                           \
        throw std::runtime_error(text);                                                                                 \
    }                                                                                                                   \

#define ASSERT_TRUE(expr, msg)                                                                                         \
    if(!bool(expr)) {                                                                                                     \
        std::string text = "ASSERTION called at " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + "\n"; \
        text = text + "Reason: " + std::to_string(expr) + "\n";                                                         \
        text = text + "Message:" + msg + "\nErrno: " + std::to_string(errno);                                           \
        throw std::runtime_error(text);                                                                                 \
    }

/** ZMQ Publisher

    Lightweight wrapper base class to initialize a ZMQ Publisher.
    Nothing data specific, but everything is only 'protected'.
    It also has an internal mutex that can be used for thread-safe
    access to the underlying connection;
**/
class ZmqPublisher {
    protected:
        const uint16_t m_port;
        std::string m_address;
        zmq::context_t m_ctx;
        zmq::socket_t m_socket;
        std::mutex g_zmq_socket;

    public:
        ZmqPublisher(std::string ip, uint16_t port, uint32_t n_threads) :
            m_port(port), m_address("tcp://*:" + std::to_string(port)), m_ctx(n_threads), m_socket(m_ctx, ZMQ_PUB) {
            // Bind the socket
            m_socket.bind(m_address.c_str());
            std::cout << "Initialized ZMQ publisher at " << m_address << std::endl;
        };

        ~ZmqPublisher(){};
};


/** ZMQ Image Publisher

    Specialized publisher to send 'ImageBinaryFormat' data format as
    multipart message. It also takes care of thread safety.

    NOTE: This method implements a single publisher! The receiver should take care of load balancing and redistributing.
**/
class ZmqImagePublisher: public ZmqPublisher {
    public:
        ZmqImagePublisher(std::string ip, uint16_t port, uint32_t n_threads) : ZmqPublisher(ip, port, n_threads) {};
        const std::string topic = "IMAGEDATA";

        std::string serializer(const NewImageMetadata& meta){
            rapidjson::Document header(rapidjson::kObjectType);
            auto& header_alloc = header.GetAllocator();

            // Fill the RapidJSON header with metadata
            header.AddMember("version", meta.version, header_alloc);
            header.AddMember("id", meta.id, header_alloc);
            header.AddMember("height", meta.height, header_alloc);
            header.AddMember("width", meta.width, header_alloc);
            header.AddMember("dtype", meta.dtype, header_alloc);
            header.AddMember("encoding", meta.encoding, header_alloc);
            header.AddMember("array_id", meta.array_id, header_alloc);
            header.AddMember("status", meta.status, header_alloc);
            header.AddMember("user_1", meta.user_1, header_alloc);
            header.AddMember("user_2", meta.user_2, header_alloc);
            // Set image shape
            auto shape_value = rapidjson::Value(rapidjson::kArrayType);
            shape_value.PushBack((uint64_t)meta.height, header_alloc);
            shape_value.PushBack((uint64_t)meta.width, header_alloc);
            header.AddMember("shape", shape_value, header_alloc);

            // Serialize header
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            header.Accept(writer);

            std::string text_header = buffer.GetString();
            return text_header;
        }

        void sendImage(ImageBinaryFormat& image){
            auto meta_str = serializer(image.meta);
            std::cout << "Metadata JSON file:\n" << meta_str << std::endl;

            std::lock_guard<std::mutex> guard(g_zmq_socket);
            int len;

            // len = m_socket.send(topic.c_str(), topic.size(), ZMQ_SNDMORE);
            // ASSERT_TRUE( len >=0, "Failed to send topic data" )
            len = m_socket.send(meta_str.c_str(), meta_str.length(), ZMQ_SNDMORE);
            ASSERT_TRUE( len >=0, "Failed to send meta data" )
            len = m_socket.send(image.data.data(), image.data.size(), 0);
            ASSERT_TRUE( len >=0, "Failed to send image data" )

            if(image.meta.id%100==0){
                std::cout << "Sent ZMQ stream of pulse: " << image.meta.id << std::endl;
            }
        }
};


#endif //SF_DAQ_BUFFER_ZMQ_IMAGE_PUBLISHER_HPP
