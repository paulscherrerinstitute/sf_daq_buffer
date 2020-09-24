#ifndef BUFFER_UTILS_HPP
#define BUFFER_UTILS_HPP

#include <string>
#include <vector>

namespace BufferUtils
{
    std::string get_filename(
            std::string detector_folder,
            std::string module_name,
            uint64_t pulse_id);

    std::size_t get_file_frame_index(uint64_t pulse_id);

    void update_latest_file(
            const std::string& latest_filename,
            const std::string& filename_to_write);

    void create_destination_folder(const std::string& output_file);

    void* bind_socket(
            void* ctx, const std::string& detector_name, const int source_id);
    void* connect_socket(
            void* ctx, const std::string& detector_name, const int source_id);
}

#endif //BUFFER_UTILS_HPP
