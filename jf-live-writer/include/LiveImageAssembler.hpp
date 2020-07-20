#ifndef SF_DAQ_BUFFER_LIVEIMAGEASSEMBLER_HPP
#define SF_DAQ_BUFFER_LIVEIMAGEASSEMBLER_HPP

#include <atomic>

#include "buffer_config.hpp"
#include "formats.hpp"

const uint64_t IA_EMPTY_SLOT_VALUE = 0;

struct ImageMetadata
{
    uint64_t pulse_id;
    uint64_t frame_index;
    uint32_t daq_rec;
    uint8_t is_good_image;
};

class LiveImageAssembler {
    const size_t n_modules_;
    const size_t image_buffer_slot_n_bytes_;

    char* image_buffer_;
    ImageMetadata* image_meta_buffer_;
    ModuleFrame* frame_meta_buffer_;
    std::atomic_int* buffer_status_;
    std::atomic_uint64_t* buffer_pulse_id_;

    size_t get_data_offset(const uint64_t slot_id, const int i_module);
    size_t get_frame_metadata_offset(const uint64_t slot_id, const int i_module);

public:
    LiveImageAssembler(const size_t n_modules);

    virtual ~LiveImageAssembler();

    bool is_slot_free(const uint64_t pulse_id);
    bool is_slot_full(const uint64_t pulse_id);

    void process(const uint64_t pulse_id,
                 const int i_module,
                 const BufferBinaryFormat* block_buffer);

    void free_slot(const uint64_t pulse_id);

    ImageMetadata* get_metadata_buffer(const uint64_t pulse_id);
    char* get_data_buffer(const uint64_t pulse_id);
};


#endif //SF_DAQ_BUFFER_LIVEIMAGEASSEMBLER_HPP
