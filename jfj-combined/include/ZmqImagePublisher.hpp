#ifndef SF_DAQ_BUFFER_ZMQ_IMAGE_PUBLISHER_HPP
#define SF_DAQ_BUFFER_ZMQ_IMAGE_PUBLISHER_HPP

#include <iostream>
#include <zmq.hpp>
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
    It also has an internal mutex that can be used for threadsafe
    access to the undelying connection;
**/
class ZmqPublisher {
    protected:
        const uint16_t m_port;
        std::string m_address;
        zmq::context_t m_ctx;
        zmq::socket_t m_socket;
        std::mutex g_zmq_socket;

    public:
        ZmqPublisher(std::string ip, uint16_t port) :
            m_port(port), m_address("tcp://*:" + std::to_string(port)), m_ctx(1), m_socket(m_ctx, ZMQ_PUB) {
            // Bind the socket
            m_socket.bind(m_address.c_str());
            std::cout << "Initialized ZMQ publisher at " << m_address << std::endl;
        };

        ~ZmqPublisher(){};
};


/** ZMQ Image Publisher

    Specialized publisher to send 'ImageBinaryFormat' data format as
    multipart message. It also takes care of thread safety.
**/
class ZmqImagePublisher: public ZmqPublisher {
    public:
        ZmqImagePublisher(std::string ip, uint16_t port) : ZmqPublisher(ip, port) {};
        const std::string topic = "IMAGEDATA";

        void sendImage(ImageBinaryFormat& image){
            std::lock_guard<std::mutex> guard(g_zmq_socket);
            int len;

            len = m_socket.send(topic.c_str(), topic.size(), ZMQ_SNDMORE);
            ASSERT_TRUE( len >=0, "Failed to send topic data" )
            len = m_socket.send(&image.meta, sizeof(image.meta), ZMQ_SNDMORE);
            ASSERT_TRUE( len >=0, "Failed to send meta data" )
            len = m_socket.send(image.data.data(), image.data.size(), 0);
            ASSERT_TRUE( len >=0, "Failed to send image data" )

            std::cout << "Sent ZMQ stream of pulse: " << image.meta.pulse_id << std::endl;
        }
};



#endif //SF_DAQ_BUFFER_ZMQ_IMAGE_PUBLISHER_HPP
