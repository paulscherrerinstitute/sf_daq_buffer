#ifndef BUFFER_UTILS_HPP
#define BUFFER_UTILS_HPP

#include <string>
#include <vector>

namespace BufferUtils
{

    struct DetectorConfig {
        const std::string streamvis_address;
        const int reduction_factor_streamvis;
        const std::string live_analysis_address;
        const int reduction_factor_live_analysis;
        const std::string PEDE_FILENAME;
        const std::string GAIN_FILENAME;
        const std::string DETECTOR_NAME;
        const int n_modules;
        const std::string pulse_address;
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
            void* ctx, const std::string& detector_name, const int source_id);
    void* connect_socket(
            void* ctx, const std::string& detector_name, const int source_id);

    DetectorConfig read_json_config(const std::string& filename);
}

#endif //BUFFER_UTILS_HPP
