#ifndef JFFILEFORMAT_HPP
#define JFFILEFORMAT_HPP

#include "jungfrau.hpp"

const char JF_FORMAT_START_BYTE = 0xBE;

#pragma pack(push)
#pragma pack(1)
struct BufferBinaryFormat {

    BufferBinaryFormat() : FORMAT_MARKER(JF_FORMAT_START_BYTE) {};

    const char FORMAT_MARKER;
    ModuleFrame metadata;
    char data[JUNGFRAU_DATA_BYTES_PER_FRAME];
};
#pragma pack(pop)

#endif // JFFILEFORMAT_HPP