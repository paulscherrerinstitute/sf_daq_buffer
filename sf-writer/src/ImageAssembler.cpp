#include <cstring>
#include "ImageAssembler.hpp"

using namespace std;
using namespace core_buffer;

ImageAssembler::ImageAssembler(const size_t n_modules) :
    n_modules_(n_modules),
    image_buffer_slot_n_bytes_(BUFFER_BLOCK_SIZE * MODULE_N_BYTES * n_modules_)
{
    image_buffer_ = new char[IA_N_SLOTS * image_buffer_slot_n_bytes_];
    image_metadata_buffer_ = new ImageMetadataBlock[IA_N_SLOTS];
    frame_metadata_buffer_ =
            new ModuleFrame[IA_N_SLOTS * BUFFER_BLOCK_SIZE * n_modules];
    buffer_status_ = new atomic_int[IA_N_SLOTS];

    for (size_t i=0; i<IA_N_SLOTS; i++) {
        free_slot(i);
    }
}

ImageAssembler::~ImageAssembler()
{
    delete[] image_buffer_;
    delete[] image_metadata_buffer_;
}

bool ImageAssembler::is_slot_free(const int bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    return buffer_status_[slot_id].load() > 0;
}

bool ImageAssembler::is_slot_full(const int bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    return buffer_status_[slot_id].load() == 0;
}

void ImageAssembler::process(
        int bunch_id,
        const int i_module,
        const BufferBinaryBlock* block_buffer)
{
    auto slot_id = bunch_id % IA_N_SLOTS;

    // TODO: Temp workaround. Make proper initialization.
    if (i_module == 0) {
        image_metadata_buffer_[slot_id].block_start_pulse_id =
                block_buffer->frame[0].metadata.pulse_id;
        image_metadata_buffer_[slot_id].block_stop_pulse_id =
                block_buffer->frame[BUFFER_BLOCK_SIZE-1].metadata.pulse_id;
    }

    auto metadata_offset =
    frame_metadata_buffer_[metadata_offset] = 0;

    size_t slot_offset = slot_id * image_buffer_slot_n_bytes_;
    size_t module_image_offset = i_module * MODULE_N_BYTES;

    for (size_t i_pulse=0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {
        size_t image_offset = i_pulse * MODULE_N_BYTES * n_modules_;

        memcpy(
            image_buffer_ + slot_offset + image_offset + module_image_offset,
            &(block_buffer->frame[i_pulse].data[0]),
            MODULE_N_BYTES);

        // TODO: Temp workaround. We need to synchronize this access.
        if (i_module == 0) {
            image_metadata_buffer_[slot_id].pulse_id[i_pulse] =
                    block_buffer->frame[i_pulse].metadata.pulse_id;
            image_metadata_buffer_[slot_id].frame_index[i_pulse] =
                    block_buffer->frame[i_pulse].metadata.frame_index;
            image_metadata_buffer_[slot_id].daq_rec[i_pulse] =
                    block_buffer->frame[i_pulse].metadata.daq_rec;
//            metadata_buffer_[slot_id].is_good_image[i_pulse] =
//                    block_buffer->frame[i_pulse].is_good_image.pulse_id;
        }
    }

    buffer_status_[slot_id].fetch_sub(1);
}

void ImageAssembler::free_slot(const int bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    buffer_status_[slot_id].store(n_modules_);
}

ImageMetadataBlock* ImageAssembler::get_metadata_buffer(const int bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    return &(image_metadata_buffer_[slot_id]);
}

char* ImageAssembler::get_data_buffer(const int bunch_id)
{
    auto slot_id = bunch_id % IA_N_SLOTS;
    return image_buffer_ + (slot_id * image_buffer_slot_n_bytes_);
}
