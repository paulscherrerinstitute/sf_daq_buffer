#ifndef FASTH5WRITER_HPP
#define FASTH5WRITER_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <H5Cpp.h>
#include <memory>
#include <unordered_map>

class BufferH5Writer {

    const uint16_t CHUNKING_FACTOR = 1;

    const size_t n_frames_per_file_;
    const uint16_t y_frame_size_;
    const uint16_t x_frame_size_;
    const size_t frame_bytes_size_;
    const std::string device_name_;
    const std::string root_folder_;
    const std::string latest_filename_;
    const std::string current_filename_;

    std::string current_output_filename_;
    H5::H5File current_output_file_;
    H5::DataSet current_image_dataset_;
    uint64_t current_pulse_id_;
    size_t current_frame_index_;

    std::unordered_map<std::string, char*> buffers_;
    std::unordered_map<std::string, H5::DataSet> datasets_;

    std::unordered_map<std::string, H5::PredType> scalar_metadata_;

    void create_file(const std::string& filename);
    void write_scalar_metadata(
            const std::string& name,
            const void* value,
            const H5::DataType data_type);

public:
    BufferH5Writer(
            const size_t n_frames_per_file,
            const uint16_t y_frame_size,
            const uint16_t x_frame_size,
            const std::string& device_name,
            const std::string& root_folder);

    virtual ~BufferH5Writer();

    template <class T> void add_scalar_metadata(
            const std::string& metadata_name);

    void set_pulse_id(const uint64_t pulse_id);

    void write_data(const char* buffer);
    template <class T> void write_scalar_metadata(
            const std::string& name,
            const void* value);

    void close_file();
};


#endif //FASTH5WRITER_HPP
