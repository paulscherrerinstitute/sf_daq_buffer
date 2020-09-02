#ifndef SF_DAQ_BUFFER_RAMBUFFER_HPP
#define SF_DAQ_BUFFER_RAMBUFFER_HPP

#include <string>
#include "formats.hpp"

class RamBuffer {
    const std::string detector_name_;
    const int module_n_;
    const size_t n_slots_;
    const size_t image_size_;
    const size_t buffer_size_;

    int shm_fd_;
    void* buffer_;

    ModuleFrame* meta_buffer_;
    char* image_buffer_;


public:
    RamBuffer(const std::string& detector_name,
              const size_t n_modules,
              const int module_n,
              const size_t n_slots);
    ~RamBuffer();

    void write_frame(const ModuleFrame& metadata, const char* data);
};


#endif //SF_DAQ_BUFFER_RAMBUFFER_HPP
