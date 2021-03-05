#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <unistd.h>
#include "RamBuffer.hpp"
#include "buffer_config.hpp"
#include "date.h"
#include <chrono>
#include <iostream>


using namespace std;
using namespace buffer_config;

RamBuffer::RamBuffer(
        const string &detector_name,
        const int n_modules,
        const int n_slots) :
        detector_name_(detector_name),
        n_modules_(n_modules),
        n_slots_(n_slots),
        meta_bytes_(sizeof(ModuleFrame) * n_modules_),
        image_bytes_(MODULE_N_BYTES * n_modules_),
        buffer_bytes_((meta_bytes_ + image_bytes_) * n_slots_)
{
    shm_fd_ = shm_open(detector_name_.c_str(), O_RDWR | O_CREAT, 0777);
    if (shm_fd_ < 0) {
        throw runtime_error(strerror(errno));
    }

    if ((ftruncate(shm_fd_, buffer_bytes_)) == -1) {
        throw runtime_error(strerror(errno));
    }

    // TODO: Test with MAP_HUGETLB
    buffer_ = mmap(NULL, buffer_bytes_, PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (buffer_ == MAP_FAILED) {
        throw runtime_error(strerror(errno));
    }

    // Metadata buffer is located at the start of the memory region.
    meta_buffer_ = (ModuleFrame *) buffer_;
    // Image buffer start right after metadata buffer.
    image_buffer_ = (char*)buffer_ + (meta_bytes_ * n_slots_);
}

RamBuffer::~RamBuffer()
{
    munmap(buffer_, buffer_bytes_);
    close(shm_fd_);
    shm_unlink(detector_name_.c_str());
}

void RamBuffer::write_frame(
        const ModuleFrame& src_meta,
        const char *src_data) const
{
    
    const int slot_n = src_meta.frame_index % n_slots_;

    ModuleFrame *dst_meta = meta_buffer_ +
                            (n_modules_ * slot_n) +
                            src_meta.module_id;

    char *dst_data = image_buffer_ +
                     (image_bytes_ * slot_n) +
                     (MODULE_N_BYTES * src_meta.module_id);

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [RamBuffer::write_frame] :";
        cout << " || src_meta.frame_index " << src_meta.frame_index;
        cout << " || src_meta.n_recv_packets " << src_meta.n_recv_packets;
        cout << " || src_meta.daq_rec " << src_meta.daq_rec;
        cout << " || src_meta.module_id " << src_meta.module_id;
        cout << " || slot_n " << slot_n;
        cout << endl;
    #endif

    memcpy(dst_meta, &src_meta, sizeof(ModuleFrame));
    memcpy(dst_data, src_data, MODULE_N_BYTES);
}

void RamBuffer::read_frame(
        const uint64_t pulse_id,
        const uint64_t module_id,
        ModuleFrame& dst_meta,
        char* dst_data) const
{
    const size_t slot_n = pulse_id % n_slots_;

    ModuleFrame *src_meta = meta_buffer_ + (n_modules_ * slot_n) + module_id;

    char *src_data = image_buffer_ +
                     (image_bytes_ * slot_n) +
                     (MODULE_N_BYTES * module_id);

    memcpy(&dst_meta, src_meta, sizeof(ModuleFrame));
    memcpy(dst_data, src_data, MODULE_N_BYTES);
}

void RamBuffer::assemble_image(
        const uint64_t pulse_id, ImageMetadata &image_meta) const
{
    const size_t slot_n = pulse_id % n_slots_;
    ModuleFrame *src_meta = meta_buffer_ + (n_modules_ * slot_n);

    auto is_pulse_init = false;
    auto is_good_image = true;

    // for each module it collects the metadata from each frame
    for (int i_module=0; i_module <  n_modules_; i_module++) {
        ModuleFrame *frame_meta = src_meta + i_module;

        #ifdef DEBUG_OUTPUT
            using namespace date;
            cout << " [" << std::chrono::system_clock::now();
            cout << "] [RamBuffer::assemble_image] :";
            cout << "module_id: " << i_module;
            cout << " || frame index: " << frame_meta->frame_index;
            cout << " || row: " << frame_meta->row;
            cout << " || column: " << frame_meta->column;
            cout << " || n_recv_packets: " << frame_meta->n_recv_packets;
            cout << endl;
        #endif

        auto is_good_frame =
                frame_meta->n_recv_packets == N_PACKETS_PER_FRAME;

        if (!is_good_frame) {
            is_good_image = false;
            #ifdef DEBUG_OUTPUT
                using namespace date;
                cout << " [" << std::chrono::system_clock::now();
                cout << "] [RamBuffer::assemble_image] ";
                cout << " not a good frame " << is_good_frame;
                cout << "n_recv_packets != N_PACKETS_PER_FRAME";
                cout << endl;
            #endif
            continue;
        }
        // in the first module will enter here and define image metadata
        if (!is_pulse_init) {
            if (frame_meta->frame_index != pulse_id) {
                stringstream err_msg;
                err_msg << "[RamBuffer::assemble_image]";
                err_msg << " Unexpected pulse_id in ram buffer.";
                err_msg << " expected=" << pulse_id;
                err_msg << " got=" << frame_meta->pulse_id;

                for (int i = 0; i < n_modules_; i++) {
                    ModuleFrame *meta = src_meta + i_module;

                    err_msg << " (module " << i << ", ";
                    err_msg << meta->pulse_id << "),";
                }
                err_msg << endl;

                throw runtime_error(err_msg.str());
            }
            
            image_meta.pulse_id = frame_meta->pulse_id;
            image_meta.frame_index = frame_meta->frame_index;
            image_meta.daq_rec = frame_meta->daq_rec;
            #ifdef DEBUG_OUTPUT
                using namespace date;
                cout << " [" << std::chrono::system_clock::now();
                cout << "] [RamBuffer::assemble_image] :";
                cout << " Defining image_meta";
                cout << " || image_meta.frame_index " << image_meta.frame_index;
                cout << endl;
            #endif
            is_pulse_init = 1;
        }

        if (is_good_image) {
            if (frame_meta->pulse_id != image_meta.pulse_id) {
                is_good_image = false;
                #ifdef DEBUG_OUTPUT
                    using namespace date;
                    cout << " [" << std::chrono::system_clock::now();
                    cout << "] [RamBuffer::assemble_image] ";
                    cout << "not good image";
                    cout << "frame_meta->pulse_id != image_meta.pulse_id";
                    cout << endl;
                #endif

                // TODO: Add some diagnostics in case this happens.
            }

            if (frame_meta->frame_index != image_meta.frame_index) {
                is_good_image = false;
                #ifdef DEBUG_OUTPUT
                    using namespace date;
                    cout << " [" << std::chrono::system_clock::now();
                    cout << "] [RamBuffer::assemble_image] !is_pulse_init:";
                    cout << "frame_meta->frame_index != image_meta.frame_index";
                    cout << endl;
                #endif
            }

            if (frame_meta->daq_rec != image_meta.daq_rec) {
                is_good_image = false;
                #ifdef DEBUG_OUTPUT
                    using namespace date;
                    cout << " [" << std::chrono::system_clock::now();
                    cout << "] [RamBuffer::assemble_image] !is_pulse_init:";
                    cout << "frame_meta->daq_rec != image_meta.daq_rec";
                    cout << endl;
                #endif
            }
        }
    }

    image_meta.is_good_image = is_good_image;

    if (!is_pulse_init) {
        image_meta.pulse_id = 0;
        image_meta.frame_index = 0;
        image_meta.daq_rec = 0;
    }
}

char* RamBuffer::read_image(const uint64_t pulse_id) const
{
    const size_t slot_n = pulse_id % n_slots_;
    char *src_data = image_buffer_ + (image_bytes_ * slot_n);

    return src_data;
}
