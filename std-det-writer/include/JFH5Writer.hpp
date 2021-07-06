#ifndef JF_LIVE_WRITER_HPP
#define JF_LIVE_WRITER_HPP

#include <memory>
#include <string>
#include <BufferUtils.hpp>
#include <formats.hpp>

extern "C" {
    #include <hdf5_hl.h>
}

class JFH5Writer {

    const std::string root_folder_;
    const std::string detector_name_;

    static const int64_t NO_RUN_ID = -1;

    // Run specific variables.
    int64_t current_run_id_ = NO_RUN_ID;
    uint32_t image_y_size_ = 0;
    uint32_t image_x_size_ = 0;
    uint32_t bits_per_pixel_ = 0;
    uint32_t image_n_bytes_ = 0;

    // Open file specific variables.
    hid_t file_id_ = -1;
    hid_t image_data_dataset_ = -1;
    hid_t image_id_dataset_ = -1;
    hid_t status_dataset_ = -1;

    static hid_t get_datatype(int bits_per_pixel);
    void open_file(const std::string& output_file, uint32_t n_images);
    void close_file();

public:
    explicit JFH5Writer(const BufferUtils::DetectorConfig& config);
    ~JFH5Writer();

    void open_run(int64_t run_id,
                  uint32_t n_images,
                  uint32_t image_y_size,
                  uint32_t image_x_size,
                  uint32_t bits_per_pixel);
    void close_run();

    void write_data(int64_t run_id,
                    uint32_t index,
                    const char* data);

    void write_meta(int64_t run_id,
                    uint32_t index,
                    const ImageMetadata& meta);
};

#endif //JF_LIVE_WRITER_HPP
