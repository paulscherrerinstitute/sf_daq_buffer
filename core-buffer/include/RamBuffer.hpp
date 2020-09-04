#ifndef SF_DAQ_BUFFER_RAMBUFFER_HPP
#define SF_DAQ_BUFFER_RAMBUFFER_HPP

#include <string>
#include "formats.hpp"

class RamBuffer {
    const std::string detector_name_;
    const int n_modules_;
    const int module_n_;

    const size_t meta_size_;
    const size_t image_size_;
    const size_t buffer_size_;

    int shm_fd_;
    void* buffer_;

    ModuleFrame* meta_buffer_;
    char* image_buffer_;

public:
    RamBuffer(const std::string& detector_name,
              const int n_modules,
              const int module_n=0);
    ~RamBuffer();

    void write_frame(const ModuleFrame *src_meta, const char *src_data) const;
    char* read_image(const uint64_t pulse_id, ImageMetadata &image_meta) const;
};


#endif //SF_DAQ_BUFFER_RAMBUFFER_HPP
