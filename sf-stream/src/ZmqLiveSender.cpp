#include "ZmqLiveSender.hpp"
#include "stream_config.hpp"

#include "zmq.h"
#include <stdexcept>

using namespace std;
using namespace stream_config;

LiveStreamConfig read_json_config(const std::string filename)
{
    std::ifstream ifs(filename);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document config_parameters;
    config_parameters.ParseStream(isw);

    return {
            config_parameters["streamvis_stream"].GetString(),
            config_parameters["streamvis_rate"].GetInt(),
            config_parameters["live_stream"].GetString(),
            config_parameters["live_rate"].GetInt(),
            config_parameters["pedestal_file"].GetString(),
            config_parameters["gain_file"].GetString(),
            config_parameters["detector_name"].GetString(),
            config_parameters["n_modules"].GetInt(),
            "tcp://127.0.0.1:51234"
    };
}

ZmqLiveSender::ZmqLiveSender(
        void* ctx,
        const LiveStreamConfig& config) :
            ctx_(ctx),
            config_(config)
{
    // TODO: Set also LINGER and SNDHWM.
    socket_streamvis_ = zmq_socket(ctx, ZMQ_PUB);

    if (zmq_bind(socket_streamvis_, config.streamvis_address.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    {
        socket_live_ = zmq_socket(ctx, ZMQ_PUSH);

        const int sndhwm = PROCESSING_ZMQ_SNDHWM;
        if (zmq_setsockopt(
                socket_live_, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }

        const int linger = 0;
        if (zmq_setsockopt(
                socket_live_, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }

        if (zmq_bind(socket_live_, config.live_analysis_address.c_str()) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }
    }

    {
        socket_pulse_ = zmq_socket(ctx, ZMQ_PUB);

        if (zmq_bind(socket_pulse_, config.pulse_address.c_str()) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }

        const int sndhwm = PULSE_ZMQ_SNDHWM;
        if (zmq_setsockopt(
                socket_pulse_, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }

        const int linger = 0;
        if (zmq_setsockopt(
                socket_pulse_, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }
    }

}

ZmqLiveSender::~ZmqLiveSender()
{
    zmq_close(socket_streamvis_);
    zmq_close(socket_live_);
}

void ZmqLiveSender::send(const ImageMetadata *meta, const char *data)
{
    uint16_t data_empty [] = { 0, 0, 0, 0};

    rapidjson::Document header(rapidjson::kObjectType);
    auto& header_alloc = header.GetAllocator();
    string text_header;

    if(zmq_send(socket_pulse_, &meta->pulse_id, sizeof(uint64_t), 0) == -1) {
        throw runtime_error(zmq_strerror(errno));
    }

    // TODO: Here we need to send to streamvis and live analysis metadata(probably need to operate still on them) and data(not every frame)
    header.AddMember("frame", meta->frame_index, header_alloc);
    header.AddMember("is_good_frame", meta->is_good_image, header_alloc);
    header.AddMember("daq_rec", meta->daq_rec, header_alloc);
    header.AddMember("pulse_id", meta->pulse_id, header_alloc);

    rapidjson::Value pedestal_file;
    pedestal_file.SetString(config_.PEDE_FILENAME.c_str(), header_alloc);
    header.AddMember("pedestal_file", pedestal_file, header_alloc);

    rapidjson::Value gain_file;
    gain_file.SetString(config_.GAIN_FILENAME.c_str(), header_alloc);
    header.AddMember("gain_file", gain_file, header_alloc);

    header.AddMember("number_frames_expected", 10000, header_alloc);

    rapidjson::Value run_name;
    run_name.SetString(
            to_string(uint64_t(meta->pulse_id/10000)*10000).c_str(),
            header_alloc);
    header.AddMember("run_name", run_name, header_alloc);

    rapidjson::Value detector_name;
    detector_name.SetString(config_.DETECTOR_NAME.c_str(), header_alloc);
    header.AddMember("detector_name", detector_name, header_alloc);

    header.AddMember("htype", "array-1.0", header_alloc);
    header.AddMember("type", "uint16", header_alloc);

    // To be retrieved and filled with correct values down.
    auto shape_value = rapidjson::Value(rapidjson::kArrayType);
    shape_value.PushBack((uint64_t)0, header_alloc);
    shape_value.PushBack((uint64_t)0, header_alloc);
    header.AddMember("shape", shape_value, header_alloc);

    int send_streamvis = 0;
    if ( config_.reduction_factor_streamvis > 1 ) {
        send_streamvis = rand() % config_.reduction_factor_streamvis;
    }
    if ( send_streamvis == 0 ) {
        auto& shape = header["shape"];
        shape[0] = config_.n_modules*512;
        shape[1] = 1024;
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
                 buffer_config::MODULE_N_BYTES * config_.n_modules,
                 0);
    } else {
        zmq_send(socket_streamvis_,
                 (char*)data_empty,
                 8,
                 0);
    }

    //same for live analysis
    int send_live_analysis = 0;
    if ( config_.reduction_factor_live_analysis > 1 ) {
        send_live_analysis = rand() % config_.reduction_factor_live_analysis;
    }
    if ( send_live_analysis == 0 ) {
        auto& shape = header["shape"];
        shape[0] = config_.n_modules*512;
        shape[1] = 1024;
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

    // TODO: Ugly. Fix this flow control.
    if (zmq_send(socket_live_,
                 text_header.c_str(),
                 text_header.size(),
                 ZMQ_SNDMORE | ZMQ_NOBLOCK) != -1) {

        if ( send_live_analysis == 0 ) {
            zmq_send(socket_live_,
                     (char*)data,
                     buffer_config::MODULE_N_BYTES * config_.n_modules,
                     ZMQ_NOBLOCK);
        } else {
            zmq_send(socket_live_,
                     (char*)data_empty,
                     8,
                     ZMQ_NOBLOCK);
        }
    }


}

