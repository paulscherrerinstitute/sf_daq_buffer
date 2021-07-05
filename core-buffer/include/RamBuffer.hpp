#ifndef SF_DAQ_BUFFER_RAMBUFFER_HPP
#define SF_DAQ_BUFFER_RAMBUFFER_HPP

#include <string>
#include "formats.hpp"
#include "buffer_config.hpp"

class RamBuffer {
    const std::string buffer_name_;
    const int n_modules_;
    const int n_slots_;

    const size_t meta_bytes_;
    const size_t data_bytes_;
    const size_t slot_bytes_;
    const size_t buffer_bytes_;

    int shm_fd_;
    char* buffer_;

private:
    char* _get_meta_buffer(int slot_n, uint64_t module_id) const;
    char* _get_frame_data_buffer(int slot_n, uint64_t module_id) const;

public:
    RamBuffer(const std::string& buffer_name,
              size_t meta_n_bytes,
              size_t data_n_bytes,
              int n_modules,
              int n_slots);

    ~RamBuffer();

    void write_frame(const ModuleFrame &src_meta, const char *src_data) const;
    void read_frame(const uint64_t pulse_id,
                     const uint64_t module_id,
                     ModuleFrame &meta,
                     char *data) const;

    char* get_frame_data(
            const uint64_t image_id, const uint64_t module_id) const;
    char* get_slot_data(const uint64_t image_id) const;
    char* get_frame_meta(
            const uint64_t image_id, const uint64_t module_id) const;
    char* get_slot_meta(const uint64_t image_id) const;


};


#endif //SF_DAQ_BUFFER_RAMBUFFER_HPP
