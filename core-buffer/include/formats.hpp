#ifndef SF_DAQ_BUFFER_FORMATS_HPP
#define SF_DAQ_BUFFER_FORMATS_HPP

#define INVALID_FRAME_INDEX UINT64_C(-1)

#pragma pack(push)
#pragma pack(1)
struct ModuleFrame {
    uint64_t id;
    uint64_t pulse_id;
    uint64_t frame_index;
    uint64_t daq_rec;
    uint64_t n_recv_packets;
    uint64_t module_id;
    uint16_t bit_depth;
    uint16_t pos_y;
    uint16_t pos_x;
};
#pragma pack(pop)

// ImageMetadata status convention
// 0 good image
// 1 frames with missing packets
// 2 frames with different ids

#pragma pack(push)
#pragma pack(1)
struct ImageMetadata {
    uint64_t version;
    uint64_t id;
    uint64_t height;
    uint64_t width;
    uint16_t dtype;
    uint16_t encoding;
    uint16_t source_id;
    uint16_t status;
    uint64_t user_1;
    uint64_t user_2;
};
#pragma pack(pop)

#endif //SF_DAQ_BUFFER_FORMATS_HPP
