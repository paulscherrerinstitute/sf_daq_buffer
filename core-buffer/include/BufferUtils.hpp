#ifndef BUFFER_UTILS_HPP
#define BUFFER_UTILS_HPP

#include <string>
#include <vector>

namespace BufferUtils
{
    std::string get_filename(
            std::string root_folder,
            std::string device_name,
            uint64_t pulse_id);

    std::size_t get_file_frame_index(uint64_t pulse_id);

    void update_latest_file(
            const std::string& latest_filename,
            const std::string& filename_to_write);

    void create_destination_folder(const std::string& output_file);
}

#endif //BUFFER_UTILS_HPP
