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
