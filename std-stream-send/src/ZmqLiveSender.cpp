#include "ZmqLiveSender.hpp"
#include "stream_config.hpp"

#include "zmq.h"
#include <stdexcept>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <iostream>
//
using namespace std;
using namespace stream_config;

ZmqLiveSender::ZmqLiveSender(
        void* ctx,
        const std::string& det_name,
        const std::string& stream_address):
            ctx_(ctx),
            det_name_(det_name),
            stream_address_(stream_address)
{
    socket_streamvis_ = BufferUtils::bind_socket(
            ctx_, det_name_, stream_address_);
}

ZmqLiveSender::~ZmqLiveSender()
{
    zmq_close(socket_streamvis_);
}

std::string ZmqLiveSender::_get_data_type_mapping(const int dtype) const{
    if (dtype == 1)
        return "uint8";
    else if (dtype == 2)
        return "uint16";
    else if (dtype == 4)
        return "uint32";
    else if (dtype == 8)
        return "uint64";
}

void ZmqLiveSender::send(const ImageMetadata& meta, const char *data,
    const size_t image_n_bytes)
{
    uint16_t data_empty [] = { 0, 0, 0, 0};

    rapidjson::Document header(rapidjson::kObjectType);
    auto& header_alloc = header.GetAllocator();
    string text_header;

    // TODO: Here we need to send to streamvis and live analysis metadata(probably need to operate still on them) and data(not every frame)
    header.AddMember("frame", meta.id, header_alloc);
    header.AddMember("is_good_frame", meta.status, header_alloc);

    rapidjson::Value detector_name;
    detector_name.SetString(det_name_.c_str(), header_alloc);
    header.AddMember("detector_name", detector_name, header_alloc);

    header.AddMember("htype", "array-1.0", header_alloc);
    
    rapidjson::Value dtype;
    dtype.SetString(_get_data_type_mapping(meta.dtype).c_str(), header_alloc);
    header.AddMember("type", dtype, header_alloc);

    // To be retrieved and filled with correct values down.
    auto shape_value = rapidjson::Value(rapidjson::kArrayType);
    shape_value.PushBack((uint64_t)0, header_alloc);
    shape_value.PushBack((uint64_t)0, header_alloc);
    header.AddMember("shape", shape_value, header_alloc);

    int send_streamvis = 0;
    send_streamvis = rand() % REDUCTION_FACTOR;
    if ( send_streamvis == 0 ) {
        auto& shape = header["shape"];
        shape[0] = meta.width;
        shape[1] = meta.height;
    } else{
        auto& shape = header["shape"];
        shape[0] = 2;
        shape[1] = 2;
    }

    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        header.Accept(writer);

        text_header = buffer.GetString();
    }

    zmq_send(socket_streamvis_,
             text_header.c_str(),
             text_header.size(),
             ZMQ_SNDMORE);

    if ( send_streamvis == 0 ) {
        zmq_send(socket_streamvis_,
                 (char*)data,
                 image_n_bytes,
                 0);
    } else {
        zmq_send(socket_streamvis_,
                 (char*)data_empty,
                 8,
                 0);
    }

}