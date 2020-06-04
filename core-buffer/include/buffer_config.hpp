#ifndef BUFFERCONFIG_HPP
#define BUFFERCONFIG_HPP

#include <cstddef>
#include <string>

namespace core_buffer {

    const size_t MODULE_X_SIZE = 1024;
    const size_t MODULE_Y_SIZE = 512;
    const size_t MODULE_N_PIXELS = MODULE_X_SIZE * MODULE_Y_SIZE;
    const size_t PIXEL_N_BYTES = 2;
    const size_t MODULE_N_BYTES = MODULE_N_PIXELS * PIXEL_N_BYTES;

    // How many frames we store in each file.
    // Must be power of 10 and <= than FOLDER_MOD
    const size_t FILE_MOD = 1000;
    // How many frames go into each files folder.
    // Must be power of 10 and >= than FILE_MOD.
    const size_t FOLDER_MOD = 100000;
    // Extension of our file format.
    const std::string FILE_EXTENSION = ".bin";
    // Number of pulses between each statistics print out.
    const size_t STATS_MODULO = 100;
    // If the RB is empty, how much time to wait before trying to read it again.
    const size_t RB_READ_RETRY_INTERVAL_MS = 5;
    // How many frames to read at once from file.
    const size_t BUFFER_BLOCK_SIZE = 100;


    const size_t BUFFER_UDP_N_RECV_MSG = 64;
    // Size of UDP recv buffer
    const int BUFFER_UDP_RCVBUF_N_SLOTS = 100;
    // 8246 bytes for each UDP packet.
    const int BUFFER_UDP_RCVBUF_BYTES =
            (128 * BUFFER_UDP_RCVBUF_N_SLOTS * 8246);
    // Microseconds timeout for UDP recv.
    const int BUFFER_UDP_US_TIMEOUT = 2 * 1000;
    // HWM for live stream from buffer.
    const int BUFFER_ZMQ_SNDHWM = 100;
    // IPC address of the live stream.
    const std::string BUFFER_LIVE_IPC_URL = "ipc:///tmp/sf-live-";

    // N of IO threads to receive data from modules.
    const int STREAM_ZMQ_IO_THREADS = 4;
    // How long should the RECV queue be.
    const size_t STREAM_RCVHWM = 100;
    // Size of buffer between the receiving and sending part.
    const int STREAM_FASTQUEUE_SLOTS = 5;
}

#endif //BUFFERCONFIG_HPP
