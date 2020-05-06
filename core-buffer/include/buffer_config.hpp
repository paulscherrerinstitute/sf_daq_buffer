#ifndef BUFFERCONFIG_HPP
#define BUFFERCONFIG_HPP

#include <cstddef>
#include <string>

namespace core_buffer {

    const size_t MODULE_X_SIZE = 1024;
    const size_t MODULE_Y_SIZE = 512;
    const size_t MODULE_N_PIXELS = MODULE_X_SIZE * MODULE_Y_SIZE;
    const size_t MODULE_N_BYTES = MODULE_N_PIXELS * 2;

    // How many frames we store in each file.
    // Must be power of 10 and <= than FOLDER_MOD
    const size_t FILE_MOD = 1000;

    // How many frames go into each files folder.
    // Must be power of 10 and >= than FILE_MOD.
    const size_t FOLDER_MOD = 100000;

    // Extension of our file format.
    const std::string FILE_EXTENSION = ".h5";

    // How many frames do we read at once during replay.
    const size_t REPLAY_READ_BLOCK_SIZE = 100;

    // How long should the RECV queue be.
    const size_t STREAM_RCV_QUEUE_SIZE = 100;

    // Size of sf_buffer RB in elements.
    const size_t BUFFER_INTERNAL_QUEUE_SIZE = 1000;

    // Time to sleep before retrying to read the queue.
    const size_t BUFFER_QUEUE_RETRY_MS = 5;

    // Microseconds timeout for UDP recv.
    const int BUFFER_UDP_US_TIMEOUT = 5 * 1000;

    // Output queue length for buffer live stream.
    const int BUFFER_LIVE_SEND_HWM = 100;

    // ZMQ threads for receiving data from sf_replay.
    const int WRITER_ZMQ_IO_THREADS = 2;

    // Size of buffer between the receiving and writing part of sf_writer
    const int WRITER_RB_BUFFER_SLOTS = 5;

    // How many frames to buffer before flushing to file.
    const int WRITER_N_FRAMES_BUFFER = 50;

    // Number of pulses between each statistics print out.
    const size_t STATS_MODULO = 100;

    // If the RB is empty, how much time to wait before trying to read it again.
    const size_t RB_READ_RETRY_INTERVAL_MS = 5;

    const size_t LIVE_READ_BLOCK_SIZE = 10;
}

#endif //BUFFERCONFIG_HPP
