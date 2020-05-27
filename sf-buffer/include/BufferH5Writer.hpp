#ifndef FASTH5WRITER_HPP
#define FASTH5WRITER_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <H5Cpp.h>
#include <memory>
#include <unordered_map>

#include "jungfrau.hpp"
#include "buffer_config.hpp"

class BufferH5Writer {

    const hsize_t meta_disk_dims[2] = {
            core_buffer::FILE_MOD,
            ModuleFrame_N_FIELDS
    };

    const hsize_t data_disk_dims[3] = {
            core_buffer::FILE_MOD,
            core_buffer::MODULE_Y_SIZE,
            core_buffer::MODULE_X_SIZE
    };

    const std::string root_folder_;
    const std::string device_name_;
    const std::string LATEST_filename_;
    const std::string CURRENT_filename_;

    std::string output_filename_;
    H5::H5File h5_file_;

    H5::DataSet current_image_dataset_;
    H5::DataSet current_metadata_dataset_;

    uint64_t current_pulse_id_;
    size_t current_file_index_;

    void create_file(const std::string& filename);


public:
    BufferH5Writer(
            const std::string& root_folder,
            const std::string& device_name);

    virtual ~BufferH5Writer();

    void set_pulse_id(const uint64_t pulse_id);

    void write(const ModuleFrame* metadata, const char* data);

    void close_file();
};


#endif //FASTH5WRITER_HPP
