#ifndef SF_DAQ_BUFFER_FORMATS_HPP
#define SF_DAQ_BUFFER_FORMATS_HPP
#include <vector>

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
    uint64_t version; // protocol version

    uint64_t id; // pulse_id for SF, frame_index for SLS
    uint64_t height; // in pixels
    uint64_t width; // in pixels

    uint16_t dtype; // enum of data types (uint8, 16, 32, float etc.)
    uint16_t encoding; // enum of encodings (raw, bshuf_lz4...)
    uint16_t array_id; // if you want to interleave 2 buffers in the same data stream
    uint16_t status; // Denotate some status of the images - corrupt for example.

    uint64_t user_1; // extra field for custom needs
    uint64_t user_2; // extra field for custom needs
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

class ImageBinaryFormat {
    public:
    ImageMetadata meta;
    std::vector<char> data;
    ImageBinaryFormat(size_t H, size_t W, size_t D): data(H*W*D, 0) { meta.height = H; meta.width = W; };
    ~ImageBinaryFormat(){ std::cout << "ImageBinaryFormat destructor called!" << std::endl; }
};

#pragma pack(push)
#pragma pack(1)
struct BufferBinaryBlock
{
    BufferBinaryFormat frame[buffer_config::BUFFER_BLOCK_SIZE];
    uint64_t start_pulse_id;
};
#pragma pack(pop)

#endif //SF_DAQ_BUFFER_FORMATS_HPP
