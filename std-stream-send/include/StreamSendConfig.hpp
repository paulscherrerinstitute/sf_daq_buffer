#ifndef SF_DAQ_BUFFER_UDPRECVCONFIG_HPP
#define SF_DAQ_BUFFER_UDPRECVCONFIG_HPP


#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <string>
#include <fstream>

struct StreamSendConfig {
    static StreamSendConfig from_json_file(const std::string& filename) {
        std::ifstream ifs(filename);
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document config_parameters;
        config_parameters.ParseStream(isw);

        return {
                config_parameters["detector_name"].GetString(),
                config_parameters["n_modules"].GetInt(),
                config_parameters["image_n_pixels"].GetInt(),
        };
    }

    const std::string detector_name;
    const int n_modules;
    const int image_n_pixels;
};


#endif //SF_DAQ_BUFFER_UDPRECVCONFIG_HPP
