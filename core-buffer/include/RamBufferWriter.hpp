#ifndef SF_DAQ_BUFFER_RAMBUFFERWRITER_HPP
#define SF_DAQ_BUFFER_RAMBUFFERWRITER_HPP

#include <string>

class RamBufferWriter {
    const int module_n_;
    const std::string detector_name_;

    int shm_fd_;

public:
    RamBufferWriter(const std::string& detector_name,
                    const int module_n,
                    const size_t n_modules,
                    const size_t n_slots);
    ~RamBufferWriter();

};


#endif //SF_DAQ_BUFFER_RAMBUFFERWRITER_HPP
