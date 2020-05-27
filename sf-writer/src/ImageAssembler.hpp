#ifndef SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
#define SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP

#include "formats.hpp"

const size_t IA_N_SLOTS = 2;

class ImageAssembler {
    const size_t n_modules_;

    char* image_buffer_;
    ImageMetadataBlock* metadata_buffer_;

public:
    ImageAssembler(const size_t n_modules);
    ImageMetadataBlock* get_metadata_buffer(const int slot_id);
    virtual ~ImageAssembler();

    void process(const int slot_id,
                 const int i_module,
                 const BufferBinaryBlock* block_buffer);
    int get_free_slot();
    int get_full_slot();
    void free_slot(const int slot_id);


    const char* get_data_buffer(const int slot_id);
};


#endif //SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
