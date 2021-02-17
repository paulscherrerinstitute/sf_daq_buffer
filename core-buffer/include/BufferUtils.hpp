#ifndef BUFFER_UTILS_HPP
#define BUFFER_UTILS_HPP

#include <string>
#include <vector>
#include <ostream>

namespace BufferUtils
{

    struct DetectorConfig {
        const std::string streamvis_address;
        const int reduction_factor_streamvis;
        const std::string live_analysis_address;
        const int reduction_factor_live_analysis;
        const std::string PEDE_FILENAME;
        const std::string GAIN_FILENAME;

        const std::string detector_name;
        const int n_modules;
        const int start_udp_port;
        const std::string buffer_folder;

        friend std::ostream& operator <<(std::ostream& os, DetectorConfig const& det_config)
        {
                return os << det_config.streamvis_address << ' '
                        << det_config.reduction_factor_streamvis << ' '
                        << det_config.live_analysis_address << ' '
                        << det_config.reduction_factor_live_analysis << ' '
                        << det_config.PEDE_FILENAME << ' '
                        << det_config.GAIN_FILENAME << ' '
                        << det_config.detector_name << ' '
                        << det_config.n_modules << ' '
                        << det_config.start_udp_port << ' '
                        << det_config.buffer_folder << ' ';
        }
    };


    std::string get_filename(
            const std::string& detector_folder,
            const std::string& module_name,
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
