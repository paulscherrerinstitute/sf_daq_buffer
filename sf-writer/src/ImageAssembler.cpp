#include <cstring>
#include "ImageAssembler.hpp"

using namespace std;
using namespace core_buffer;

ImageAssembler::ImageAssembler(const size_t n_modules) :
    n_modules_(n_modules),
    image_buffer_slot_n_bytes_(BUFFER_BLOCK_SIZE * MODULE_N_BYTES * n_modules_)
{
    image_buffer_ = new char[IA_N_SLOTS * image_buffer_slot_n_bytes_];
    meta_buffer_ = new ImageMetadataBlock[IA_N_SLOTS];
    frame_metadata_buffer_ =
            new ModuleFrame[IA_N_SLOTS * n_modules * BUFFER_BLOCK_SIZE];
    buffer_status_ = new atomic_int[IA_N_SLOTS];

    for (size_t i=0; i<IA_N_SLOTS; i++) {
        free_slot(i);
    }
}

ImageAssembler::~ImageAssembler()
{
    delete[] image_buffer_;
    delete[] meta_buffer_;
}

bool ImageAssembler::is_slot_free(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    return buffer_status_[slot_id].load() > 0;
}

bool ImageAssembler::is_slot_full(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    return buffer_status_[slot_id].load() == 0;
}

void ImageAssembler::process(
        const uint64_t bunch_id,
        const int i_module,
        const BufferBinaryBlock* block_buffer)
{
    auto slot_id = bunch_id % IA_N_SLOTS;

    // TODO: This offsets are not really readable.
    size_t slot_i_offset = slot_id * image_buffer_slot_n_bytes_;
    size_t module_i_offset = i_module * MODULE_N_BYTES;

    size_t n_metadata_in_slot = n_modules_ * BUFFER_BLOCK_SIZE;
    size_t slot_m_offset = slot_id * n_metadata_in_slot;
    size_t module_m_offset = i_module;

    for (size_t i_pulse=0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {
        size_t metadata_offset = slot_m_offset + module_m_offset;
        metadata_offset += i_pulse * n_modules_;
        memcpy(
            &(frame_metadata_buffer_[metadata_offset]),
            &(block_buffer->frame[i_pulse].metadata),
            sizeof(ModuleFrame));

        size_t image_offset = slot_i_offset + module_i_offset;
        image_offset += i_pulse * MODULE_N_BYTES * n_modules_;
        memcpy(
            image_buffer_ + image_offset,
            &(block_buffer->frame[i_pulse].data[0]),
            MODULE_N_BYTES);
    }

    buffer_status_[slot_id].fetch_sub(1);
}

void ImageAssembler::free_slot(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    buffer_status_[slot_id].store(n_modules_);
}

ImageMetadataBlock* ImageAssembler::get_metadata_buffer(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;

    for (size_t i_pulse=0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {
        auto is_not_init = false;

        for (size_t i_module=0; i_module < n_modules_; i_module++) {

//
//            if (is_not_init) {
//                if ()
//            }
        }

        meta_buffer_[slot_id].pulse_id[i_pulse];
    }

    return &(meta_buffer_[slot_id]);
}

char* ImageAssembler::get_data_buffer(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    return image_buffer_ + (slot_id * image_buffer_slot_n_bytes_);
}
