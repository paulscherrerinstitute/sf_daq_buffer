#ifndef SF_DAQ_BUFFER_UDPRECVCONFIG_HPP
#define SF_DAQ_BUFFER_UDPRECVCONFIG_HPP


#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <string>
#include <fstream>

struct UdpRecvConfig {
    static UdpRecvConfig from_json_file(const std::string& filename) {
        std::ifstream ifs(filename);
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document config_parameters;
        config_parameters.ParseStream(isw);

        return {
                config_parameters["detector_name"].GetString(),
                config_parameters["detector_type"].GetString(),
                config_parameters["n_modules"].GetInt(),
                config_parameters["start_udp_port"].GetInt(),
        };
    }

    const std::string detector_name;
    const std::string detector_type;
    const int n_modules;
    const int start_udp_port;
};


#endif //SF_DAQ_BUFFER_UDPRECVCONFIG_HPP
