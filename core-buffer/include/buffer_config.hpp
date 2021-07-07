#ifndef BUFFERCONFIG_HPP
#define BUFFERCONFIG_HPP

#include <cstddef>
#include <string>

namespace buffer_config {

    // How many frames we store in each file.
    // Must be power of 10 and <= than FOLDER_MOD
    const size_t FILE_MOD = 1000;
    // How many frames go into each files folder.
    // Must be power of 10 and >= than FILE_MOD.
    const size_t FOLDER_MOD = 100000;
    // Extension of our file format.
    const std::string FILE_EXTENSION = ".bin";
    // Number of pulses between each statistics print out (buffer_writer, stream2vis...)
    const size_t STATS_MODULO = 1000;
    // Number of seconds after which statistics is print out (udp_recv)
    const size_t STATS_TIME = 10;
    // If the RB is empty, how much time to wait before trying to read it again.
    const size_t RB_READ_RETRY_INTERVAL_MS = 5;
    // How many frames to read at once from file.
    const size_t BUFFER_BLOCK_SIZE = 100;


    // Size of UDP recv buffer
    const int BUFFER_UDP_RCVBUF_N_SLOTS = 100;
    // 8246 bytes for each UDP packet.
    const int BUFFER_UDP_RCVBUF_BYTES =
            (128 * BUFFER_UDP_RCVBUF_N_SLOTS * 8246);
    // Microseconds timeout for UDP recv.
    const int BUFFER_UDP_US_TIMEOUT = 2 * 1000;
    // HWM for live stream from buffer.
    const int BUFFER_ZMQ_SNDHWM = 100;
    // HWM for live stream from buffer.
    const int BUFFER_ZMQ_RCVHWM = 100;
    // IPC address of the live stream.
    const std::string IPC_URL_BASE = "ipc:///tmp/std-daq-";
    // Number of image slots in ram buffer - 10 seconds should be enough
    const int RAM_BUFFER_N_SLOTS = 100 * 10;
}

#endif //BUFFERCONFIG_HPP
