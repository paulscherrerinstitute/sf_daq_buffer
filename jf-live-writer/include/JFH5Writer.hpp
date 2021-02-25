#ifndef SFWRITER_HPP
#define SFWRITER_HPP

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
    const uint32_t image_y_size_;
    const uint32_t image_x_size_;

    static const int64_t NO_RUN_ID;
    int64_t current_run_id_ = NO_RUN_ID;

    hid_t file_id_ = -1;
    hid_t image_dataset_id_ = -1;
    hid_t pulse_dataset_id_= -1;
    hid_t frame_dataset_id_ = -1;
    hid_t daq_rec_dataset_id_ = -1;
    hid_t is_good_dataset_id_ = -1;

    void open_file(const std::string& output_file, const uint32_t n_images);
    void close_file();

public:
    JFH5Writer(const BufferUtils::DetectorConfig config);
    ~JFH5Writer();
    void open_run(const int64_t run_id, const uint32_t n_images);
    void close_run();

    void write_data(const int64_t run_id,
                    const uint32_t index,
                    const char* data);

    void write_meta(const int64_t run_id,
                    const uint32_t index,
                    const ImageMetadata& meta);
};

#endif //SFWRITER_HPP
