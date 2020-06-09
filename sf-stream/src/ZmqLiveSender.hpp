#ifndef SF_DAQ_BUFFER_ZMQLIVESENDER_HPP
#define SF_DAQ_BUFFER_ZMQLIVESENDER_HPP

#include <string>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "formats.hpp"


struct LiveStreamConfig {
    const std::string streamvis_address;
    const int reduction_factor_streamvis;
    const std::string live_analysis_address;
    const int reduction_factor_live_analysis;
    const std::string PEDE_FILENAME;
    const std::string GAIN_FILENAME;
    const std::string DETECTOR_NAME;
    const int n_modules;
};

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
        config_parameters["n_modules"].GetInt()
    };
}

class ZmqLiveSender {
    const void* ctx_;
    const LiveStreamConfig config_;

    void* socket_streamvis_;
    void* socket_live_;

public:
    ZmqLiveSender(void* ctx,
                  const LiveStreamConfig& config);
    ~ZmqLiveSender();

    void send(const ModuleFrameBuffer* meta, const char* data);
};


#endif //SF_DAQ_BUFFER_ZMQLIVESENDER_HPP
