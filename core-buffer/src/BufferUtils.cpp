#include "BufferUtils.hpp"

#include <sstream>
#include <buffer_config.hpp>
#include <zmq.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include "date.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>

using namespace std;
using namespace buffer_config;

string BufferUtils::get_image_filename(
        const std::string& detector_folder,
        const uint64_t pulse_id)
{
    uint64_t data_folder = pulse_id / buffer_config::FOLDER_MOD;
    data_folder *= buffer_config::FOLDER_MOD;

    uint64_t data_file = pulse_id / buffer_config::FILE_MOD;
    data_file *= buffer_config::FILE_MOD;

    stringstream folder;
    folder << detector_folder << "/";
    folder << data_folder << "/";
    folder << data_file << buffer_config::FILE_EXTENSION;

    return folder.str();
}

string BufferUtils::get_filename(
        const std::string& detector_folder,
        const std::string& module_name,
        const uint64_t pulse_id)
{
    uint64_t data_folder = pulse_id / buffer_config::FOLDER_MOD;
    data_folder *= buffer_config::FOLDER_MOD;

    uint64_t data_file = pulse_id / buffer_config::FILE_MOD;
    data_file *= buffer_config::FILE_MOD;

    stringstream folder;
    folder << detector_folder << "/";
    folder << module_name << "/";
    folder << data_folder << "/";
    folder << data_file << buffer_config::FILE_EXTENSION;

    return folder.str();
}

size_t BufferUtils::get_file_frame_index(const uint64_t pulse_id)
{
    uint64_t file_base = pulse_id / buffer_config::FILE_MOD;
    file_base *= buffer_config::FILE_MOD;

    return pulse_id - file_base;
}

void BufferUtils::update_latest_file(
        const std::string& latest_filename,
        const std::string& filename_to_write)
{
    // TODO: Ugly hack, please please fix it.
    // TODO: This for now works only if the root_folder is absolute path.

    stringstream latest_command;
    latest_command << "echo " << filename_to_write;
    latest_command << " > " << latest_filename;
    auto str_latest_command = latest_command.str();

    system(str_latest_command.c_str());
}

void BufferUtils::create_destination_folder(const string& output_file)
{
    auto file_separator_index = output_file.rfind('/');

    if (file_separator_index != string::npos) {

        string output_folder(output_file.substr(0, file_separator_index));

        // TODO: filesystem::create_directories(output_folder)
        string create_folder_command("mkdir -p " + output_folder);
        system(create_folder_command.c_str());
    }
}

void* BufferUtils::connect_socket(
        void* ctx, const string& detector_name, const string& stream_name)
{
    string ipc_address = buffer_config::IPC_URL_BASE +
                         detector_name + "-" +
                         stream_name;
    
    void* socket = zmq_socket(ctx, ZMQ_SUB);
    if (socket == nullptr) {
        throw runtime_error(zmq_strerror(errno));
    }

    int rcvhwm = BUFFER_ZMQ_RCVHWM;
    if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    int linger = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_connect(socket, ipc_address.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    return socket;
}

void* BufferUtils::bind_socket(
        void* ctx, const string& detector_name, const string& stream_name)
{
    string ipc_address = IPC_URL_BASE +
                         detector_name + "-" +
                         stream_name;

    void* socket = zmq_socket(ctx, ZMQ_PUB);

    const int sndhwm = BUFFER_ZMQ_SNDHWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    const int linger = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_bind(socket, ipc_address.c_str()) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    return socket;
}

BufferUtils::DetectorConfig BufferUtils::read_json_config(
        const std::string& filename)
{
    std::ifstream ifs(filename);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document config_parameters;
    config_parameters.ParseStream(isw);

    return {
            config_parameters["detector_name"].GetString(),
            config_parameters["detector_type"].GetString(),
            config_parameters["n_modules"].GetInt(),
            config_parameters["image_height"].GetInt(),
            config_parameters["image_width"].GetInt(),
            config_parameters["start_udp_port"].GetInt(),
    };
}
