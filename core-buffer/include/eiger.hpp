#ifndef EIGER_H
#define EIGER_H

#include <cstdint>

#define N_MODULES 1
#define BYTES_PER_PACKET 1072
#define DATA_BYTES_PER_PACKET 1024
// DR 16
#define N_PACKETS_PER_FRAME 256
#define DATA_BYTES_PER_FRAME 262144
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
    uint16_t xCoord;
    uint16_t yCoord;
    uint16_t zCoord;

    uint32_t debug;
    uint16_t roundRobin;
    uint8_t detectortype;
    uint8_t headerVersion;
    char data[DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)


#endif
