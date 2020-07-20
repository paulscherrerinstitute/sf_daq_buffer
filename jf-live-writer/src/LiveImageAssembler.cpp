#include <cstring>

#include "LiveImageAssembler.hpp"
#include "buffer_config.hpp"
#include "live_writer_config.hpp"

using namespace std;
using namespace buffer_config;
using namespace live_writer_config;

LiveImageAssembler::LiveImageAssembler(const size_t n_modules) :
        n_modules_(n_modules),
        image_buffer_slot_n_bytes_(MODULE_N_BYTES * n_modules_)
{
    image_buffer_ = new char[WRITER_IA_N_SLOTS * image_buffer_slot_n_bytes_];
    image_meta_buffer_ = new ImageMetadata[WRITER_IA_N_SLOTS];
    frame_meta_buffer_ = new ModuleFrame[WRITER_IA_N_SLOTS * n_modules];
    buffer_status_ = new atomic_int[WRITER_IA_N_SLOTS];
    buffer_pulse_id_ = new atomic_uint64_t[WRITER_IA_N_SLOTS];

    for (size_t i=0; i < WRITER_IA_N_SLOTS; i++) {
        free_slot(i);
    }
}

LiveImageAssembler::~LiveImageAssembler()
{
    delete[] image_buffer_;
    delete[] image_meta_buffer_;
}

bool LiveImageAssembler::is_slot_free(const uint64_t pulse_id)
{
    auto slot_id = pulse_id % WRITER_IA_N_SLOTS;

    uint64_t slot_pulse_id = IA_EMPTY_SLOT_VALUE;
    if (buffer_pulse_id_[slot_id].compare_exchange_strong(
            slot_pulse_id, pulse_id)) {
        return true;
    }

    auto is_free = buffer_status_[slot_id].load(memory_order_relaxed) > 0;
    return is_free && (slot_pulse_id == pulse_id);
}

bool LiveImageAssembler::is_slot_full(const uint64_t pulse_id)
{
    auto slot_id = pulse_id % WRITER_IA_N_SLOTS;
    return buffer_status_[slot_id].load(memory_order_relaxed) == 0;
}

size_t LiveImageAssembler::get_data_offset(
        const uint64_t slot_id, const int i_module)
{
    size_t slot_i_offset = slot_id * image_buffer_slot_n_bytes_;
    size_t module_i_offset = i_module * MODULE_N_BYTES;

    return slot_i_offset + module_i_offset;
}

size_t LiveImageAssembler::get_frame_metadata_offset(
        const uint64_t slot_id, const int i_module)
{
    size_t slot_m_offset = slot_id * n_modules_;
    size_t module_m_offset = i_module;

    return slot_m_offset + module_m_offset;
}

void LiveImageAssembler::process(
        const uint64_t pulse_id,
        const int i_module,
        const BufferBinaryFormat* file_buffer)
{
    const auto slot_id = pulse_id % WRITER_IA_N_SLOTS;

    auto frame_meta_offset = get_frame_metadata_offset(slot_id, i_module);
    auto image_offset = get_data_offset(slot_id, i_module);

    memcpy(
            &(frame_meta_buffer_[frame_meta_offset]),
            &(file_buffer->metadata),
            sizeof(file_buffer->metadata));

    memcpy(
            image_buffer_ + image_offset,
            &(file_buffer->data[0]),
            MODULE_N_BYTES);

    buffer_status_[slot_id].fetch_sub(1, memory_order_relaxed);
}

void LiveImageAssembler::free_slot(const uint64_t pulse_id)
{
    auto slot_id = pulse_id % WRITER_IA_N_SLOTS;
    buffer_status_[slot_id].store(n_modules_, memory_order_relaxed);
    buffer_pulse_id_[slot_id].store(IA_EMPTY_SLOT_VALUE, memory_order_relaxed);
}

ImageMetadata* LiveImageAssembler::get_metadata_buffer(const uint64_t pulse_id)
{
    const auto slot_id = pulse_id % WRITER_IA_N_SLOTS;

    ImageMetadata& image_meta = image_meta_buffer_[slot_id];

    auto frame_meta_offset = get_frame_metadata_offset(slot_id, 0);

    auto is_pulse_init = false;
    image_meta.is_good_image = 1;
    image_meta.pulse_id = 0;

    for (size_t i_module=0; i_module < n_modules_; i_module++) {

        auto& frame_meta = frame_meta_buffer_[frame_meta_offset];
        frame_meta_offset += 1;

        auto is_good_frame =
                frame_meta.n_recv_packets == JF_N_PACKETS_PER_FRAME;

        if (!is_good_frame) {
            image_meta.pulse_id = 0;
            continue;
        }

        if (!is_pulse_init) {
            image_meta.pulse_id = frame_meta.pulse_id;
            image_meta.frame_index = frame_meta.frame_index;
            image_meta.daq_rec = frame_meta.daq_rec;

            is_pulse_init = true;
        }

        if (image_meta.is_good_image == 1) {
            if (frame_meta.pulse_id != image_meta.pulse_id) {
                image_meta.is_good_image = 0;
            }

            if (frame_meta.frame_index !=  image_meta.frame_index) {
                image_meta.is_good_image = 0;
            }

            if (frame_meta.daq_rec != image_meta.daq_rec) {
                image_meta.is_good_image = 0;
            }

            if (frame_meta.n_recv_packets != JF_N_PACKETS_PER_FRAME) {
                image_meta.is_good_image = 0;
            }
        }
    }

    return &image_meta;
}

char* LiveImageAssembler::get_data_buffer(const uint64_t pulse_id)
{
    auto slot_id = pulse_id % WRITER_IA_N_SLOTS;
    return image_buffer_ + (slot_id * image_buffer_slot_n_bytes_);
}
