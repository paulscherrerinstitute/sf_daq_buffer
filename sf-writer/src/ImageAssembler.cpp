#include <cstring>
#include "ImageAssembler.hpp"

using namespace std;
using namespace core_buffer;

ImageAssembler::ImageAssembler(const size_t n_modules) :
    n_modules_(n_modules),
    image_buffer_slot_n_bytes_(BUFFER_BLOCK_SIZE * MODULE_N_BYTES * n_modules_)
{
    image_buffer_ = new char[IA_N_SLOTS * image_buffer_slot_n_bytes_];
    metadata_buffer_ = new ImageMetadataBlock[IA_N_SLOTS];
}

ImageAssembler::~ImageAssembler()
{
    delete[] image_buffer_;
    delete[] metadata_buffer_;
}

int ImageAssembler::get_free_slot()
{

}

void ImageAssembler::process(
        const int slot_id,
        const int i_module,
        const BufferBinaryBlock* block_buffer)
{
    size_t slot_offset = slot_id * image_buffer_slot_n_bytes_;
    size_t module_image_offset = i_module * MODULE_N_BYTES;

    for (size_t i_pulse=0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {
        size_t image_offset = i_pulse * MODULE_N_BYTES * n_modules_;

        memcpy(
            image_buffer_ + slot_offset + image_offset + module_image_offset,
            &(block_buffer->frame[i_pulse].data[0]),
            MODULE_N_BYTES);


    }
}

int ImageAssembler::get_full_slot()
{

}

void ImageAssembler::free_slot(const int slot_id)
{

}

ImageMetadataBlock* ImageAssembler::get_metadata_buffer(const int slot_id)
{
    return &(metadata_buffer_[slot_id]);
}

char* ImageAssembler::get_data_buffer(const int slot_id)
{
    return image_buffer_ + (slot_id * image_buffer_slot_n_bytes_);
}
