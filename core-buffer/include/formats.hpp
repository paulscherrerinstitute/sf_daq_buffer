#ifndef SF_DAQ_BUFFER_FORMATS_HPP
#define SF_DAQ_BUFFER_FORMATS_HPP

#include "buffer_config.hpp"
#include "jungfrau.hpp"

#pragma pack(push)
#pragma pack(1)
struct ModuleFrame {
    uint64_t pulse_id;
    uint64_t frame_index;
    uint64_t daq_rec;
    uint64_t n_recv_packets;
    uint64_t module_id;
};
#pragma pack(pop)


#pragma pack(push)
#pragma pack(1)
struct ImageMetadata {
    uint64_t pulse_id;
    uint64_t frame_index;
    uint32_t daq_rec;
    uint32_t is_good_image;
};
#pragma pack(pop)

struct ModuleFrameBuffer {
    ModuleFrame module[JUNGFRAU_N_MODULES];
};

#pragma pack(push)
#pragma pack(1)
struct BufferBinaryFormat {
    const char FORMAT_MARKER = 0xBE;
    ModuleFrame meta;
    char data[buffer_config::MODULE_N_BYTES];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct BufferBinaryBlock
{
    BufferBinaryFormat frame[buffer_config::BUFFER_BLOCK_SIZE];
    uint64_t start_pulse_id;
};
#pragma pack(pop)

#endif //SF_DAQ_BUFFER_FORMATS_HPP
