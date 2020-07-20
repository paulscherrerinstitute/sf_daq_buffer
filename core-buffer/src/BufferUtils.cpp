#include "BufferUtils.hpp"

#include <sstream>
#include <buffer_config.hpp>

using namespace std;

string BufferUtils::get_filename(
        std::string detector_folder,
        std::string module_name,
        uint64_t pulse_id)
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

size_t BufferUtils::get_file_frame_index(uint64_t pulse_id)
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