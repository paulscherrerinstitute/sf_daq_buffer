#ifndef EIGER_H
#define EIGER_H

#include <cstdint>

#define N_MODULES 1
#define BYTES_PER_PACKET 4144
#define DATA_BYTES_PER_PACKET 4096

// EIGER 1M
// Each packet line is made of 2 chip lines -> [CHIP1]<gap>[CHIP2]
// gaps between chips
// For a 1M:
// 0 gap 1
// gap  gap
// 2 gap 3

#define MODULE_X_SIZE 256
#define MODULE_Y_SIZE 512
#define MODULE_N_PIXELS 131072
#define PIXEL_N_BYTES 2
#define MODULE_N_BYTES 262144
#define GAP_X_MODULE_PIXELS 2
#define GAP_Y_MODULE_PIXELS 2
#define GAP_X_EIGERMOD_PIXELS 8
#define GAP_Y_EIGERMOD_PIXELS 36


#define N_BYTES_PER_MODULE_LINE(bit_depth) ((MODULE_X_SIZE * bit_depth) / 8)
// #define N_BYTES_PER_IMAGE_LINE(bit_depth, n_submodules) ((n_submodules / 2 * MODULE_X_SIZE * bit_depth) / 8)

// DR 16
// #define N_PACKETS_PER_FRAME 256
// #define DATA_BYTES_PER_FRAME 262144
// DR 32
// #define N_PACKETS_PER_FRAME 512
// #define DATA_BYTES_PER_FRAME 524288

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
    uint16_t reserved;

    uint32_t debug;
    uint16_t roundRobin;
    uint8_t detectortype;
    uint8_t headerVersion;
    char data[DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)


#endif
