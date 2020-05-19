#include "BufferedFastQueue.hpp"
#include <thread>

using namespace std;
using namespace core_buffer;

BufferedFastQueue::BufferedFastQueue(
        FastQueue<ImageMetadataBuffer>& queue,
        const size_t buffer_n_pulses,
        const size_t n_modules) :
            buffer_n_pulses_(buffer_n_pulses),
            queue_(queue),
            n_modules_(n_modules)
{
    while ((current_slot_id_ = queue_.reserve()) == -1){
        this_thread::sleep_for(chrono::milliseconds(5));
    }

    queue_meta_buffer_ = queue_.get_metadata_buffer(current_slot_id_);
    queue_meta_buffer_->n_pulses_in_buffer = 0;
    queue_data_buffer_ = queue_.get_data_buffer(current_slot_id_);
}

ImageMetadata* BufferedFastQueue::get_metadata_buffer()
{
    return &image_metadata_;
}

char* BufferedFastQueue::get_data_buffer()
{
    auto index = queue_meta_buffer_->n_pulses_in_buffer;
    auto image_size = MODULE_N_BYTES * n_modules_;

    return queue_data_buffer_ + (index * image_size);
}

void BufferedFastQueue::commit()
{
    auto index = queue_meta_buffer_->n_pulses_in_buffer;

    queue_meta_buffer_->pulse_id[index] = image_metadata_.pulse_id;
    queue_meta_buffer_->frame_index[index] = image_metadata_.frame_index;
    queue_meta_buffer_->daq_rec[index] = image_metadata_.daq_rec;
    queue_meta_buffer_->is_good_frame[index] = image_metadata_.is_good_frame;
    queue_meta_buffer_->data_n_bytes[index] = image_metadata_.data_n_bytes;

    queue_meta_buffer_->n_pulses_in_buffer++;

    if (queue_meta_buffer_->n_pulses_in_buffer == buffer_n_pulses_) {
        queue_.commit();

        while ((current_slot_id_ = queue_.reserve()) == -1){
            this_thread::sleep_for(chrono::milliseconds(5));
        }

        queue_meta_buffer_ = queue_.get_metadata_buffer(current_slot_id_);
        queue_meta_buffer_->n_pulses_in_buffer = 0;
        queue_data_buffer_ = queue_.get_data_buffer(current_slot_id_);
    }
}

void BufferedFastQueue::finalize() {
    if (queue_meta_buffer_->n_pulses_in_buffer > 0) {
        queue_.commit();
    }
}