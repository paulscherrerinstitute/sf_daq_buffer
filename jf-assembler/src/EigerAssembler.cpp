#include <cstdint>
#include <chrono>
#include <iostream>
#include <formats.hpp>
#include <cstring>

#include "EigerAssembler.hpp"
#include "buffer_config.hpp"
#include "eiger.hpp"
#include "date.h"

using namespace std;
using namespace buffer_config;

EigerAssembler::EigerAssembler(const int n_modules, const int bit_depth):
    n_modules_(n_modules), 
    n_eiger_modules_(n_modules/4),
    bit_depth_(bit_depth),
    n_bytes_per_frame_(MODULE_N_PIXELS * bit_depth / 8),
    n_bytes_per_frame_line_(N_BYTES_PER_MODULE_LINE(bit_depth)),
    n_packets_per_frame_(n_bytes_per_frame_ / DATA_BYTES_PER_PACKET),
    n_bytes_per_x_gap_(GAP_X_MODULE_PIXELS * bit_depth / 8),
    n_bytes_per_y_gap_(GAP_Y_MODULE_PIXELS * bit_depth / 8),
    n_bytes_per_eiger_x_gap_(GAP_X_EIGERMOD_PIXELS * bit_depth / 8),
    n_bytes_per_eiger_y_gap_(GAP_Y_EIGERMOD_PIXELS * bit_depth / 8),
    n_bytes_per_image_line_(n_bytes_per_frame_line_ * 2 + n_bytes_per_x_gap_),
    n_lines_per_frame_(DATA_BYTES_PER_PACKET / n_bytes_per_frame_line_ * n_packets_per_frame_),
    image_bytes_((n_modules_ * n_bytes_per_frame_) + (2 * (MODULE_Y_SIZE * 2) * bit_depth_ / 8) + ((n_eiger_modules_) * (2 * n_bytes_per_image_line_)))
{
    
}

int EigerAssembler::get_last_img_status() const
{
    return last_image_status_;
}

size_t EigerAssembler::get_module_n_bytes() const
{
    return n_bytes_per_frame_;
}

size_t EigerAssembler::get_image_n_bytes() const
{
    return image_bytes_;
}

void EigerAssembler::assemble_image(const char* src_meta,
        const char* src_data,
        char* dst_meta,
        char* dst_data)
{
    auto is_pulse_init = false;
    for (int i_module = 0; i_module < n_modules_; i_module++) {
        // module frame metadata
        auto frame_meta = (ModuleFrame *)(src_meta + (sizeof(ModuleFrame)* i_module));
        // module frame data
        auto *frame_data = src_data + (n_bytes_per_frame_ * i_module);
        // image metadata
        auto image_meta = (ImageMetadata *) dst_meta;
        // initializes image metadata
        if (!is_pulse_init){
            // init good image status = 0 
            image_meta->status = 0;
            image_meta->id = frame_meta->id;
            image_meta->height = n_modules_ / 2 * MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS;
            image_meta->width = n_modules_ / 2 * MODULE_X_SIZE + GAP_X_MODULE_PIXELS;
            image_meta->dtype = (bit_depth_ <= 8) ? 1 : bit_depth_ / 8;
            image_meta->encoding = 0;
            image_meta->source_id = 0;
            // TODO: proper user ids
            image_meta->user_1 = 0;
            image_meta->user_2 = 0;
            is_pulse_init = 1;
            #ifdef DEBUG_OUTPUT
                using namespace date;
                cout << " [" << std::chrono::system_clock::now();
                cout << "] [EigerAssembler::assemble_image] is_pulse_init ";
                cout << " || Image id: " << image_meta->id;
                cout << endl;
            #endif
        }

        // missing packets: bad status = 1
        if (frame_meta->n_recv_packets != n_packets_per_frame_){
            image_meta->status = 1;
        }
        
        // frame id false: bad status = 2
        if (frame_meta->frame_index != image_meta->id) {
            image_meta->status = 2;
        }

        // top
        uint32_t source_offset = 0;
        uint32_t reverse_factor = 0;
        uint32_t line_number = 0;
        uint32_t dest_offset = 0;

        // If bottom -> reversed
        const auto reverse = IS_BOTTOM(frame_meta->pos_x);
        if (reverse == -1) {
            line_number = MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS;
            reverse_factor = MODULE_Y_SIZE - 1;
            dest_offset += n_bytes_per_image_line_ *
                                (MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS);
            source_offset = (MODULE_Y_SIZE-1) * n_bytes_per_frame_line_;
        }

        const auto i_module_row = frame_meta->pos_x;
        const auto i_module_column = frame_meta->pos_y;


        uint32_t dest_module_line = line_number;

        if (i_module_column == 1) {
            dest_offset += n_bytes_per_frame_line_ + n_bytes_per_x_gap_;
        }

        int counter = 0;

        for (uint32_t frame_line = 0;
             frame_line < n_lines_per_frame_; frame_line++) {
            // void * destination, const void * source, size_t num 
            memcpy (
                    (char*)(dst_data + dest_offset),
                    (char*)(src_data + source_offset),
                    n_bytes_per_frame_line_
            );

            counter += 1;
            source_offset += reverse * n_bytes_per_module_line_;
            dest_offset += reverse * n_bytes_per_image_line_;
            }
        line_number += n_lines_per_frame_;
        dest_module_line = line_number + n_lines_per_frame_ - 1;

        // last module sets the last_image_status_
        if (i_module == n_modules_ - 1){
            last_image_status_ = image_meta->status;
            #ifdef DEBUG_OUTPUT
                using namespace date;
                cout << " [" << std::chrono::system_clock::now();
                cout << "] [EigerAssembler::assemble_image] ";
                cout << " || Image id: " << image_meta->id;
                cout << " || last_image_status_ " << last_image_status_;
                cout << endl;
            #endif
        }
    }
}
