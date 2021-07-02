#ifndef SF_DAQ_BUFFER_RAMBUFFER_HPP
#define SF_DAQ_BUFFER_RAMBUFFER_HPP

#include <string>
#include "formats.hpp"

class RamBuffer {
    const std::string detector_name_;
    const int n_modules_;
    const int n_submodules_;
    const int n_slots_;
    const int bit_depth_;

    const size_t n_packets_per_frame_;
    const size_t data_bytes_per_frame_;

    const size_t meta_bytes_;
    const size_t image_bytes_;
    const size_t buffer_bytes_;

    int shm_fd_;
    void* buffer_;

    ModuleFrame* meta_buffer_;
    char* image_buffer_;

private:    
    void assemble_eiger_image(ImageMetadata &image_meta, 
            const int bit_depth, const size_t slot_n) const;

public:
    RamBuffer(const std::string& detector_name,
              const int n_modules,
              const int n_submodules,
              const int bit_depth);
    ~RamBuffer();

    void write_frame(const ModuleFrame &src_meta, const char *src_data) const;
    void read_frame(const uint64_t pulse_id,
                     const uint64_t module_id,
                     ModuleFrame &meta,
                     char *data) const;
    char* read_image(const uint64_t pulse_id) const;
    void assemble_image(
            const uint64_t pulse_id, ImageMetadata &image_meta) const;
    
    

};


#endif //SF_DAQ_BUFFER_RAMBUFFER_HPP
