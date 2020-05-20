#ifndef JFFILEFORMAT_HPP
#define JFFILEFORMAT_HPP

#include "jungfrau.hpp"

const char JF_FORMAT_START_BYTE = 0xBE;

#pragma pack(push)
#pragma pack(1)
struct BufferBinaryFormat {

    BufferBinaryFormat() : FORMAT_MARKER(JF_FORMAT_START_BYTE) {};

    const char FORMAT_MARKER;
    uint64_t pulse_id;
    uint64_t frame_id;
    uint32_t daq_rec;
    uint16_t n_recv_packets;
    char data[JUNGFRAU_DATA_BYTES_PER_FRAME];
};
#pragma pack(pop)

#endif // JFFILEFORMAT_HPP