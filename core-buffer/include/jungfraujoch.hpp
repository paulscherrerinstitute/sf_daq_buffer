#ifndef JUNGFRAUJOCH_HPP
#define JUNGFRAUJOCH_HPP

#include <cstdint>

#define JFJOCH_N_MODULES 32
#define JFJOCH_BYTES_PER_PACKET 8240
#define JFJOCH_DATA_BYTES_PER_PACKET 8192
#define JFJOCH_N_PACKETS_PER_FRAME (JFJOCH_N_MODULES * 128)
#define JFJOCH_DATA_BYTES_PER_FRAME (JFJOCH_N_MODULES * 1048576)

// 48 bytes + 8192 bytes = 8240 bytes
#pragma pack(push)
#pragma pack(2)
struct jfjoch_packet_t {
    uint64_t framenum;
    uint32_t exptime;
    uint32_t packetnum;

    int64_t bunchid;
    uint64_t timestamp;

    uint16_t moduleID;
    uint16_t xCoord;
    uint16_t yCoord;
    uint16_t zCoord;

    uint32_t debug;
    uint16_t roundRobin;
    uint8_t detectortype;
    uint8_t headerVersion;
    char data[JFJOCH_DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)


#endif
