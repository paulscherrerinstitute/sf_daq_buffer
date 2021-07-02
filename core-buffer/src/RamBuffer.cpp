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
        const int n_submodules,
        const int bit_depth) :
        detector_name_(detector_name),
        n_modules_(n_modules),
        n_submodules_(n_submodules_),
        bit_depth_(bit_depth),
        n_slots_(buffer_config::RAM_BUFFER_N_SLOTS),
        meta_bytes_(sizeof(ModuleFrame) * n_modules_),
        n_packets_per_frame_(bit_depth_ * MODULE_N_PIXELS / 8 / DATA_BYTES_PER_PACKET / n_modules * n_submodules),
        data_bytes_per_frame_(n_packets_per_frame_ * DATA_BYTES_PER_PACKET),
        image_bytes_(MODULE_N_BYTES * n_modules_),
        buffer_bytes_((meta_bytes_ + image_bytes_) * n_slots_)
{

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [RamBuffer::Constructor] :";
        cout << " || bit_depth_ " << bit_depth_;
        cout << " || n_packets_per_frame_ " << n_packets_per_frame_;
        cout << " || image_bytes_ " << image_bytes_;
        cout << " || data_bytes_per_frame_ " << data_bytes_per_frame_;
        cout << endl;
    #endif
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
                     (data_bytes_per_frame_ * src_meta.module_id);

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
    memcpy(dst_data, src_data, data_bytes_per_frame_);
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
    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [RamBuffer::assemble_image] :";
        cout << " pulse_id : " << pulse_id;
    cout << endl;
    #endif

    auto is_pulse_init = false;
    auto is_good_image = true;
    
    // for each module it collects the metadata from each frame
    // and verifies if is a good frame
    for (int i_module=0; i_module < n_modules_; i_module++) {
        ModuleFrame *frame_meta = src_meta + i_module;

        #ifdef DEBUG_OUTPUT
            using namespace date;
            cout << " [" << std::chrono::system_clock::now();
            cout << "] module_id: " << i_module;
            cout << " || frame index: " << frame_meta->frame_index;
            cout << " || pulse index: " << frame_meta->pulse_id;
            cout << " || row: " << frame_meta->row;
            cout << " || column: " << frame_meta->column;
            cout << " || n_recv_packets: " << frame_meta->n_recv_packets;
            cout << endl;
        #endif

        auto is_good_frame =
                frame_meta->n_recv_packets == n_packets_per_frame_;

        if (!is_good_frame) {
            is_good_image = false;
            #ifdef DEBUG_OUTPUT
                using namespace date;
                cout << " [" << std::chrono::system_clock::now();
                cout << "] [RamBuffer::assemble_image] ";
                cout << " not a good frame " << is_good_frame;
                cout << "n_recv_packets != n_packets_per_frame_";
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

    // bad practice, what'd be the nicest way to do this?
    #ifdef USE_EIGER
        if (is_good_image){
            assemble_eiger_image(image_meta, bit_depth_, slot_n);
        }
    #endif

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

void RamBuffer::assemble_eiger_image(ImageMetadata &image_meta, const int bit_depth, const size_t slot_n) const
{
    
    const uint32_t n_bytes_per_module_line = N_BYTES_PER_MODULE_LINE(bit_depth);
    const uint32_t n_bytes_per_gap = GAP_X_MODULE_PIXELS * bit_depth / 8;
    const uint32_t n_bytes_per_image_line = n_bytes_per_module_line * 2 + n_bytes_per_gap;
    const int n_lines_per_frame = DATA_BYTES_PER_PACKET / n_bytes_per_module_line * n_packets_per_frame_;
   
    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [RamBuffer::is_good_image] ";
        cout << " image_meta.frame_index" << image_meta.frame_index;
        cout << " Verification: " << endl;
        cout << " || n_bytes_per_module_line " << n_bytes_per_module_line << endl;
        cout << " || n_bytes_per_gap " << n_bytes_per_gap << endl;
        cout << " || n_bytes_per_image_line " << n_bytes_per_image_line;
        cout << " || n_lines_per_frame " << n_lines_per_frame << endl;;
    #endif
    for (int i_module=0; i_module < n_modules_; i_module++) {
        // defines from which eiger module this submodule is part
        const int eiger_module_index = i_module / n_submodules_; 
        // module frame metadata
        ModuleFrame *frame_meta = meta_buffer_ + (n_modules_ * slot_n) + i_module;
        // module frame data
        char *frame_data = image_buffer_ + (image_bytes_ * slot_n) + (data_bytes_per_frame_ * i_module);
        // top 
        uint32_t source_offset = 0;
        uint32_t reverse_factor = 0;
        uint32_t bottom_offset = 0;
        uint32_t line_number = 0;
        uint32_t dest_line_offset=0;
        
        // If bottom -> reversed
        const auto reverse = IS_BOTTOM(frame_meta->row);
        if (reverse == -1){
            line_number = MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS;
            reverse_factor = MODULE_Y_SIZE - 1;
            dest_line_offset += n_bytes_per_image_line * (MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS);
        }

        const auto i_module_row = frame_meta->row;
        const auto i_module_column = frame_meta->column;
        
        uint32_t dest_module_line = line_number; 

        if (i_module_column == 1){
            source_offset += MODULE_X_SIZE + GAP_X_MODULE_PIXELS;
            dest_line_offset += n_bytes_per_module_line + n_bytes_per_gap;
        }

        for (uint32_t frame_line=0; frame_line < n_lines_per_frame; frame_line++)
        {
            // Copy each chip line individually, to allow a gap of n_bytes_per_chip_gap in the destination memory.
            memcpy (
                (char*)(image_buffer_ + (image_bytes_ * slot_n)) + dest_line_offset, 
                (char*) frame_data + source_offset, 
                n_bytes_per_module_line
            );

            memcpy (
                (char*)(image_buffer_ + (image_bytes_ * slot_n)) + dest_line_offset + n_bytes_per_module_line,
                (char*) frame_data + source_offset + n_bytes_per_module_line, 
                n_bytes_per_module_line
            );

            source_offset += n_bytes_per_module_line;
            dest_line_offset += reverse * n_bytes_per_image_line;
        }
        line_number += n_lines_per_frame;
        dest_module_line = line_number + n_lines_per_frame - 1;
    }
    
}