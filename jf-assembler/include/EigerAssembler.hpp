
#ifndef SF_DAQ_BUFFER_EIGERASSEMBLER_HPP
#define SF_DAQ_BUFFER_EIGERASSEMBLER_HPP


#include <cstddef>

class EigerAssembler {
    const int n_modules_;
    const int bit_depth_;

    const uint32_t n_bytes_per_frame_;
    const uint32_t n_bytes_per_module_line_;
    const uint32_t n_packets_per_frame_;
    const uint32_t n_bytes_per_gap_;
    const uint32_t n_bytes_per_image_line_;
    const int n_lines_per_frame_;

public:
    EigerAssembler(const int n_modules, const int bit_depth);
    void assemble_image(const char* src_meta, const char* src_data,
                        char* dst_meta, char* dst_data) const;
    size_t get_image_n_bytes();
};


#endif //SF_DAQ_BUFFER_EIGERASSEMBLER_HPP
