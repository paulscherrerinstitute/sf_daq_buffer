#ifndef SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
#define SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP

#include <atomic>

#include "buffer_config.hpp"
#include "formats.hpp"

const uint64_t IA_EMPTY_SLOT_VALUE = 0;

struct ImageMetadataBlock
{
    uint64_t pulse_id[buffer_config::BUFFER_BLOCK_SIZE];
    uint64_t frame_index[buffer_config::BUFFER_BLOCK_SIZE];
    uint32_t daq_rec[buffer_config::BUFFER_BLOCK_SIZE];
    uint8_t is_good_image[buffer_config::BUFFER_BLOCK_SIZE];
    uint64_t block_start_pulse_id;
    uint64_t block_stop_pulse_id;
};

class ImageAssembler {
    const size_t n_modules_;
    const size_t image_buffer_slot_n_bytes_;

    char* image_buffer_;
    ImageMetadataBlock* meta_buffer_;
    ModuleFrame* frame_meta_buffer_;
    std::atomic_int* buffer_status_;
    std::atomic_uint64_t* buffer_bunch_id_;

    size_t get_data_offset(const uint64_t slot_id, const int i_module);
    size_t get_metadata_offset(const uint64_t slot_id, const int i_module);

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
