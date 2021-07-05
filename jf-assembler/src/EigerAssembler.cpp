#include <cstdint>
#include <chrono>
#include <iostream>
#include <formats.hpp>
#include <cstring>

#include "EigerAssembler.hpp"
#include "eiger.hpp"
#include "date.h"

using namespace std;
using namespace buffer_config;

EigerAssembler::EigerAssembler(const int n_modules, const int bit_depth):
    n_modules_(n_modules), 
    n_eiger_modules_(n_modules/4),
    bit_depth_(bit_depth),
    n_bytes_per_frame_(MODULE_N_PIXELS * bit_depth / 8),
    n_bytes_per_module_line_(N_BYTES_PER_MODULE_LINE(bit_depth)),
    n_packets_per_frame_(n_bytes_per_frame_ / DATA_BYTES_PER_PACKET),
    n_bytes_per_x_gap_(GAP_X_MODULE_PIXELS * bit_depth / 8),
    n_bytes_per_y_gap_(GAP_Y_MODULE_PIXELS * bit_depth / 8),
    n_bytes_per_eiger_x_gap_(GAP_X_EIGERMOD_PIXELS * bit_depth / 8),
    n_bytes_per_eiger_y_gap_(GAP_Y_EIGERMOD_PIXELS * bit_depth / 8),
    n_bytes_per_image_line_(n_bytes_per_module_line_ * 2 + n_bytes_per_x_gap_),
    n_lines_per_frame_(DATA_BYTES_PER_PACKET / n_bytes_per_module_line_
                       * n_packets_per_frame_),
    image_bytes_((n_modules_ * MODULE_N_PIXELS * bit_depth_ / 8) +  ((n_eiger_modules_) * (MODULE_Y_SIZE * 2) * bit_depth_ / 8) + ((n_eiger_modules_) * (2 * n_bytes_per_image_line_)))
{
    
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
        char* dst_data) const
{

    for (int i_module = 0; i_module < n_modules_; i_module++) {
        // module frame metadata
        auto frame_meta = (ModuleFrame *)(src_meta + (sizeof(ModuleFrame)* i_module));
        // module frame data
        auto *frame_data = src_data + (n_bytes_per_frame_ * i_module);

        // top
        uint32_t source_offset = 0;
        uint32_t reverse_factor = 0;
        uint32_t line_number = 0;
        uint32_t dest_offset = 0;

        // If bottom -> reversed
        const auto reverse = IS_BOTTOM(frame_meta->row);
        if (reverse == -1) {
            line_number = MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS;
            reverse_factor = MODULE_Y_SIZE - 1;
            dest_offset += n_bytes_per_image_line_ *
                                (MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS);
            source_offset = (MODULE_Y_SIZE-1) * n_bytes_per_module_line_;
        }

        const auto i_module_row = frame_meta->row;
        const auto i_module_column = frame_meta->column;

        uint32_t dest_module_line = line_number;

        if (i_module_column == 1) {
            dest_offset += n_bytes_per_module_line_ + n_bytes_per_x_gap_;
        }

        #ifdef DEBUG_OUTPUT
            using namespace date;
            // if (i_module == 1){
                cout << " [" << std::chrono::system_clock::now();
                cout << "] [MODULE " << i_module;
                cout << "] (row " << i_module_row;
                cout << " , column)" << i_module_column;
                cout << " || reverse_factor" << reverse_factor;
                cout << " || line_number" << line_number;
                cout << endl;
            // }
        #endif
        int counter = 0;
        for (uint32_t frame_line = 0;
             frame_line < n_lines_per_frame_; frame_line++) {
            // void * destination, const void * source, size_t num 
            memcpy (
                    (char*)(dst_data + dest_offset),
                    (char*) (src_data + source_offset),
                    n_bytes_per_module_line_
            );
            
            #ifdef DEBUG_OUTPUT
                using namespace date;
                // verifies the addresses for 
                // beginning and end of each frame
                if (counter < 5 || counter > 508){
                    cout << " [" << std::chrono::system_clock::now();
                    cout << "] [MODULE :" << i_module;
                    cout << "] ROW :" << i_module_row;
                    cout << "] COLUMN :" << i_module_column;
                    cout << " || source_offset" << source_offset;
                    cout << " || dest_offset " << dest_offset;
                    cout << " || frame_line " << frame_line;
                    cout << " || COUNTER " << counter;
                    cout << endl;
                }
            #endif
            counter += 1;
            source_offset += reverse * n_bytes_per_module_line_;
            dest_offset += reverse * n_bytes_per_image_line_;
            }
        line_number += n_lines_per_frame_;
        dest_module_line = line_number + n_lines_per_frame_ - 1;
    }
}
