#ifndef SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
#define SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP

#include <atomic>

#include "formats.hpp"

const size_t IA_N_SLOTS = 2;

class ImageAssembler {
    const size_t n_modules_;
    const size_t image_buffer_slot_n_bytes_;

    char* image_buffer_;
    ImageMetadataBlock* meta_buffer_;
    ModuleFrame* frame_meta_buffer_;
    std::atomic_int* buffer_status_;

public:
    ImageAssembler(const size_t n_modules);

    virtual ~ImageAssembler();

    bool is_slot_free(const uint64_t bunch_id);
    bool is_slot_full(const uint64_t bunch_id);

    void process(uint64_t bunch_id,
                 const int i_module,
                 const BufferBinaryBlock* block_buffer);

    void free_slot(const uint64_t bunch_id);

    ImageMetadataBlock* get_metadata_buffer(const uint64_t bunch_id);
    char* get_data_buffer(const uint64_t bunch_id);
};


#endif //SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
