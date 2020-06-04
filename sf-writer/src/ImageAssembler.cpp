#include <cstring>

#include "ImageAssembler.hpp"
#include "writer_config.hpp"
#include "buffer_config.hpp"

using namespace std;
using namespace writer_config;
using namespace buffer_config;

ImageAssembler::ImageAssembler(const size_t n_modules) :
    n_modules_(n_modules),
    image_buffer_slot_n_bytes_(BUFFER_BLOCK_SIZE * MODULE_N_BYTES * n_modules_)
{
    image_buffer_ = new char[WRITER_IA_N_SLOTS * image_buffer_slot_n_bytes_];
    meta_buffer_ = new ImageMetadataBlock[WRITER_IA_N_SLOTS];
    frame_meta_buffer_ =
            new ModuleFrame[WRITER_IA_N_SLOTS * n_modules * BUFFER_BLOCK_SIZE];
    buffer_status_ = new atomic_int[WRITER_IA_N_SLOTS];
    buffer_bunch_id_ = new atomic_uint64_t[WRITER_IA_N_SLOTS];

    for (size_t i=0; i < WRITER_IA_N_SLOTS; i++) {
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
    auto slot_id = bunch_id % WRITER_IA_N_SLOTS;

    uint64_t slot_bunch_id = IA_EMPTY_SLOT_VALUE;
    if (buffer_bunch_id_[slot_id].compare_exchange_strong(
            slot_bunch_id, bunch_id)) {
        return true;
    }

    auto is_free = buffer_status_[slot_id].load(memory_order_relaxed) > 0;
    return is_free && (slot_bunch_id == bunch_id);
}

bool ImageAssembler::is_slot_full(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % WRITER_IA_N_SLOTS;
    return buffer_status_[slot_id].load(memory_order_relaxed) == 0;
}

size_t ImageAssembler::get_data_offset(
        const uint64_t slot_id, const int i_module)
{
    size_t slot_i_offset = slot_id * image_buffer_slot_n_bytes_;
    size_t module_i_offset = i_module * MODULE_N_BYTES;

    return slot_i_offset + module_i_offset;
}

size_t ImageAssembler::get_metadata_offset(
        const uint64_t slot_id, const int i_module)
{
    size_t n_metadata_in_slot = n_modules_ * BUFFER_BLOCK_SIZE;
    size_t slot_m_offset = slot_id * n_metadata_in_slot;
    size_t module_m_offset = i_module;

    return slot_m_offset + module_m_offset;
}

void ImageAssembler::process(
        const uint64_t bunch_id,
        const int i_module,
        const BufferBinaryBlock* block_buffer)
{
    const auto slot_id = bunch_id % WRITER_IA_N_SLOTS;

    auto meta_offset = get_metadata_offset(slot_id, i_module);
    const auto meta_offset_step = n_modules_;

    auto image_offset = get_data_offset(slot_id, i_module);
    const auto image_offset_step = MODULE_N_BYTES * n_modules_;

    for (const auto& frame : block_buffer->frame) {

        memcpy(
            &(frame_meta_buffer_[meta_offset]),
            &(frame.metadata),
            sizeof(ModuleFrame));

        meta_offset += meta_offset_step;

        memcpy(
            image_buffer_ + image_offset,
            &(frame.data[0]),
            MODULE_N_BYTES);

        image_offset += image_offset_step;
    }

    buffer_status_[slot_id].fetch_sub(1, memory_order_relaxed);
}

void ImageAssembler::free_slot(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % WRITER_IA_N_SLOTS;
    buffer_status_[slot_id].store(n_modules_, memory_order_relaxed);
    buffer_bunch_id_[slot_id].store(IA_EMPTY_SLOT_VALUE, memory_order_relaxed);
}

ImageMetadataBlock* ImageAssembler::get_metadata_buffer(const uint64_t bunch_id)
{
    const auto slot_id = bunch_id % WRITER_IA_N_SLOTS;

    auto& image_pulse_id = meta_buffer_[slot_id].pulse_id;
    auto& image_frame_index = meta_buffer_[slot_id].frame_index;
    auto& image_daq_rec = meta_buffer_[slot_id].daq_rec;
    auto& image_is_good_frame = meta_buffer_[slot_id].is_good_image;

    auto meta_offset = get_metadata_offset(slot_id, 0);
    const auto meta_offset_step = 1;

    uint64_t start_pulse_id = bunch_id * BUFFER_BLOCK_SIZE;
    meta_buffer_[slot_id].block_start_pulse_id = start_pulse_id;

    uint64_t stop_pulse_id = start_pulse_id + BUFFER_BLOCK_SIZE - 1;
    meta_buffer_[slot_id].block_stop_pulse_id = stop_pulse_id;

    for (size_t i_pulse=0; i_pulse < BUFFER_BLOCK_SIZE; i_pulse++) {

        auto is_pulse_init = false;
        image_is_good_frame[i_pulse] = 1;

        for (size_t i_module=0; i_module < n_modules_; i_module++) {

            auto& frame_meta = frame_meta_buffer_[meta_offset];
            auto is_good_frame =
                    frame_meta.n_recv_packets == JF_N_PACKETS_PER_FRAME;

            if (!is_good_frame) {
                image_is_good_frame[i_pulse] = 0;
                // TODO: Update meta_offset only once in the loop.
                meta_offset += meta_offset_step;
                continue;
            }

            if (!is_pulse_init) {
                image_pulse_id[i_pulse] = frame_meta.pulse_id;
                image_frame_index[i_pulse] = frame_meta.frame_index;
                image_daq_rec[i_pulse] = frame_meta.daq_rec;

                is_pulse_init = true;
            }

            if (image_is_good_frame[i_pulse] == 1) {
                if (frame_meta.pulse_id != image_pulse_id[i_pulse]) {
                    image_is_good_frame[i_pulse] = 0;
                }

                if (frame_meta.frame_index != image_frame_index[i_pulse]) {
                    image_is_good_frame[i_pulse] = 0;
                }

                if (frame_meta.daq_rec != image_daq_rec[i_pulse]) {
                    image_is_good_frame[i_pulse] = 0;
                }

                if (frame_meta.n_recv_packets != JF_N_PACKETS_PER_FRAME) {
                    image_is_good_frame[i_pulse] = 0;
                }
            }

            meta_offset += meta_offset_step;
        }
    }

    return &(meta_buffer_[slot_id]);
}

char* ImageAssembler::get_data_buffer(const uint64_t bunch_id)
{
    auto slot_id = bunch_id % WRITER_IA_N_SLOTS;
    return image_buffer_ + (slot_id * image_buffer_slot_n_bytes_);
}
