#ifndef JUNGFRAU_H
#define JUNGFRAU_H

#define JUNGFRAU_BYTES_PER_PACKET 8246
#define JUNGFRAU_DATA_BYTES_PER_PACKET 8192
#define JUNGFRAU_N_PACKETS_PER_FRAME 128
#define JUNGFRAU_DATA_BYTES_PER_FRAME 1048576

// 6 bytes + 48 bytes + 8192 bytes = 8246 bytes
#pragma pack(push)
#pragma pack(2)
struct jungfrau_packet {
    char emptyheader[6];
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
    char data[JUNGFRAU_DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct ModuleFrame {
    uint64_t pulse_id;
    uint64_t frame_index;
    uint64_t daq_rec;
    uint64_t n_received_packets;
    uint64_t module_id;
};
#pragma pack(pop)

#endif