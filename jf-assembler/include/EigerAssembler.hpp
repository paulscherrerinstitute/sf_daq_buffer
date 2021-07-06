
#ifndef SF_DAQ_BUFFER_EIGERASSEMBLER_HPP
#define SF_DAQ_BUFFER_EIGERASSEMBLER_HPP

#include <iostream>
#include <cstddef>

class EigerAssembler {
    const int n_modules_;
    const int bit_depth_;
    const int n_eiger_modules_;

    int last_image_status_;

    const uint32_t n_bytes_per_frame_;
    const uint32_t n_bytes_per_module_line_;
    const uint32_t n_packets_per_frame_;
    const uint32_t n_bytes_per_x_gap_;
    const uint32_t n_bytes_per_y_gap_;
    const uint32_t n_bytes_per_eiger_x_gap_;
    const uint32_t n_bytes_per_eiger_y_gap_;
    const uint32_t n_bytes_per_image_line_;
    const int n_lines_per_frame_;
    const int image_bytes_;

    

public:
    EigerAssembler(const int n_modules, const int bit_depth);
    void assemble_image(const char* src_meta, const char* src_data,
                        char* dst_meta, char* dst_data);
    size_t get_image_n_bytes() const;
    size_t get_module_n_bytes() const;
    int get_last_img_status() const;

    friend std::ostream& operator<<(std::ostream& os, const EigerAssembler& p)
    {
        return os << "( n_bytes_per_frame_"
                << p.n_bytes_per_frame_ << ", n_bytes_per_module_line_"
                << p.n_bytes_per_module_line_ << ", n_packets_per_frame_"
                << p.n_packets_per_frame_ << ", n_bytes_per_x_gap_"
                << p.n_bytes_per_x_gap_  << ", n_bytes_per_y_gap_"
                << p.n_bytes_per_y_gap_  << ", n_bytes_per_eiger_x_gap_"
                << p.n_bytes_per_eiger_x_gap_  << ", n_bytes_per_eiger_y_gap_"
                << p.n_bytes_per_eiger_y_gap_  << ", n_bytes_per_image_line_"
                << p.n_bytes_per_image_line_  << ", n_lines_per_frame_"
                << p.n_lines_per_frame_ << ", image_bytes_"
                << p.image_bytes_ << ", n_modules_"
                << p.n_modules_ << ", bit_depth_"
                << p.bit_depth_ << ""
                << ")";
    }

};


#endif //SF_DAQ_BUFFER_EIGERASSEMBLER_HPP
