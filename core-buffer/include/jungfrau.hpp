#ifndef JUNGFRAU_H
#define JUNGFRAU_H

#include <cstdint>

const std::string DETECTOR_TYPE = "jungfrau";

#define N_MODULES 32
#define BYTES_PER_PACKET 8240
#define DATA_BYTES_PER_PACKET 8192

#define MODULE_X_SIZE 1024
#define MODULE_Y_SIZE 512
#define MODULE_N_PIXELS 524288
#define PIXEL_N_BYTES 2
#define MODULE_N_BYTES 1048576

// #define N_PACKETS_PER_FRAME 128
// #define DATA_BYTES_PER_FRAME 1048576

// 48 bytes + 8192 bytes = 8240 bytes
#pragma pack(push)
#pragma pack(2)
struct det_packet {
    uint64_t framenum;
    uint32_t exptime;
    uint32_t packetnum;

    double bunchid;
    uint64_t timestamp;

    uint16_t moduleID;
    uint16_t row;
    uint16_t column;
    uint16_t zCoord;

    uint32_t debug;
    uint16_t roundRobin;
    uint8_t detectortype;
    uint8_t headerVersion;
    char data[DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)

#endif
