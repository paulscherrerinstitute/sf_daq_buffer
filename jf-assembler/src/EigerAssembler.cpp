#include <cstdint>
#include "EigerAssembler.hpp"
#include "eiger.hpp"
#include "date.h"
#include <chrono>
#include <iostream>
#include <formats.hpp>
#include <cstring>

using namespace std;

EigerAssembler::EigerAssembler(const int n_modules, const int bit_depth):
    n_modules_(n_modules),
    bit_depth_(bit_depth),
    n_bytes_per_frame_(MODULE_N_PIXELS * bit_depth / 8),
    n_bytes_per_module_line_(N_BYTES_PER_MODULE_LINE(bit_depth)),
    // TODO: Is this correct?
    n_packets_per_frame_(n_bytes_per_frame_ / DATA_BYTES_PER_PACKET),
    n_bytes_per_gap_(GAP_X_MODULE_PIXELS * bit_depth / 8),
    n_bytes_per_image_line_(n_bytes_per_module_line_ * 2 + n_bytes_per_gap_),
    n_lines_per_frame_(DATA_BYTES_PER_PACKET / n_bytes_per_module_line_
                       * n_packets_per_frame_)
{
#ifdef DEBUG_OUTPUT
    using namespace date;
    cout << " [" << std::chrono::system_clock::now();
    cout << "] [EigerAssembler::EigerAssembler] " << endl;
    cout << " || bit_depth " << bit_depth_ << endl;
    cout << endl;
#endif
}

size_t EigerAssembler::get_image_n_bytes()
{
    // TODO: Calculate the real size.
    return 0;
}

void EigerAssembler::assemble_image(
        const char* src_meta,
        const char* src_data,
        char* dst_meta,
        char* dst_data) const
{
    for (int i_module = 0; i_module < n_modules_; i_module++) {
        // defines from which eiger module this submodule is part
        // TODO: Deviding with n_modules_ .. is this correct?
        const int eiger_module_index = i_module / n_modules_;
        // module frame metadata
        // TODO: Using i_module.. is this correct?
        auto *frame_meta =
                (ModuleFrame *) src_meta + (sizeof(ModuleFrame) * i_module);
        // module frame data
        // TODO: Using i_module.. is this correct?
        auto *frame_data = src_data + (n_bytes_per_frame_ * i_module);

        // top
        uint32_t source_offset = 0;
        uint32_t reverse_factor = 0;
        uint32_t bottom_offset = 0;
        uint32_t line_number = 0;
        uint32_t dest_line_offset = 0;

        // If bottom -> reversed
        const auto reverse = IS_BOTTOM(frame_meta->row);
        if (reverse == -1) {
            line_number = MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS;
            reverse_factor = MODULE_Y_SIZE - 1;
            dest_line_offset += n_bytes_per_image_line_ *
                                (MODULE_Y_SIZE + GAP_Y_MODULE_PIXELS);
        }

        const auto i_module_row = frame_meta->row;
        const auto i_module_column = frame_meta->column;

        uint32_t dest_module_line = line_number;

        if (i_module_column == 1) {
            source_offset += MODULE_X_SIZE + GAP_X_MODULE_PIXELS;
            dest_line_offset += n_bytes_per_module_line_ + n_bytes_per_gap_;
        }

        for (uint32_t frame_line = 0;
             frame_line < n_lines_per_frame_; frame_line++) {
            // Copy each chip line individually, to allow a gap of n_bytes_per_chip_gap in the destination memory.
//            memcpy (
//                    (char*)(dst_data + (image_bytes_ * slot_n)) + dest_line_offset,
//                    // TODO: Using source_offset which has also GAP_X_MODULE in frames buffer -> is this correct?
//                    (char*) frame_data + source_offset,
//                    n_bytes_per_module_line_
//            );
//
//            memcpy (
//                    (char*)(dst_data + (image_bytes_ * slot_n)) + dest_line_offset + n_bytes_per_module_line,
//                     // TODO: Using source_offset which has also GAP_X_MODULE in frames buffer -> is this correct?
//                    (char*) frame_data + source_offset + n_bytes_per_module_line_,
//                    n_bytes_per_module_line_
//            );

            source_offset += n_bytes_per_module_line_;
            dest_line_offset += reverse * n_bytes_per_image_line_;
        }
        line_number += n_lines_per_frame_;
        dest_module_line = line_number + n_lines_per_frame_ - 1;
    }
}
