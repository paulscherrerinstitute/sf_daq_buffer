#ifndef SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
#define SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP

#include <atomic>

#include "formats.hpp"

const size_t IA_N_SLOTS = 2;

class ImageAssembler {
    const size_t n_modules_;
    const size_t image_buffer_slot_n_bytes_;

    char* image_buffer_;
    ImageMetadataBlock* metadata_buffer_;
    std::atomic_int* buffer_status_;

public:
    ImageAssembler(const size_t n_modules);

    virtual ~ImageAssembler();

    bool is_slot_free(const int bunch_id);
    bool is_slot_full(const int bunch_id);

    void process(int bunch_id,
                 const int i_module,
                 const BufferBinaryBlock* block_buffer);

    void free_slot(const int bunch_id);

    ImageMetadataBlock* get_metadata_buffer(const int slot_id);
    char* get_data_buffer(const int slot_id);
};


#endif //SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
