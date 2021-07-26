#ifndef BUFFER_UTILS_HPP
#define BUFFER_UTILS_HPP

#include <string>
#include <vector>
#include <ostream>

namespace BufferUtils
{

    struct DetectorConfig {
        const std::string detector_name;
        const std::string detector_type;
        const int n_modules;
        const int image_height;
        const int image_width;
        const int start_udp_port;
        

        friend std::ostream& operator <<(std::ostream& os, DetectorConfig const& det_config)
        {
                return os << det_config.detector_name << ' '
                        << det_config.detector_type << ' '
                        << det_config.n_modules << ' '
                        << det_config.start_udp_port << ' '
                        << det_config.image_height << ' '
                        << det_config.image_width << ' ';
        }
    };


    std::string get_filename(
            const std::string& detector_folder,
            const std::string& module_name,
            const uint64_t pulse_id);

    std::string get_image_filename(
            const std::string& detector_folder,
            const uint64_t pulse_id);

    std::size_t get_file_frame_index(const uint64_t pulse_id);

    void update_latest_file(
            const std::string& latest_filename,
            const std::string& filename_to_write);

    void create_destination_folder(const std::string& output_file);

    void* bind_socket(
            void* ctx,
            const std::string& detector_name,
            const std::string& stream_name);

    void* connect_socket(
            void* ctx,
            const std::string& detector_name,
            const std::string& stream_name);

    DetectorConfig read_json_config(const std::string& filename);
}

#endif //BUFFER_UTILS_HPP
