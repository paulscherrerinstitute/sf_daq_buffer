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


    const size_t BUFFER_UDP_N_RECV_MSG = 64;
    // Size of UDP recv buffer
    const int BUFFER_UDP_RCVBUF_N_SLOTS = 100;
    // +1 for packet headers.
    const int BUFFER_UDP_RCVBUF_BYTES =
            (128 * BUFFER_UDP_RCVBUF_N_SLOTS * 8246);
    // Microseconds timeout for UDP recv.
    const int BUFFER_UDP_US_TIMEOUT = 2 * 1000;
    // HWM for live stream from buffer.
    const int BUFFER_ZMQ_SNDHWM = 100;
    // Name of the dataset where we store the images.
    const std::string BUFFER_H5_FRAME_DATASET = "image";
    // Name of the dataset where we store metadata.
    const std::string BUFFER_H5_METADATA_DATASET = "metadata";
    // IPC address of the live stream.
    const std::string BUFFER_LIVE_IPC_URL = "ipc:///tmp/sf-live-";

    // N of IO threads to receive data from modules.
    const int STREAM_ZMQ_IO_THREADS = 4;
    // How long should the RECV queue be.
    const size_t STREAM_RCVHWM = 100;
    // Size of buffer between the receiving and sending part.
    const int STREAM_FASTQUEUE_SLOTS = 5;

    // Address where Replay streams and writer receives.
    const std::string REPLAY_STREAM_IPC_URL = "ipc:///tmp/sf-replay-";
    // How many frames to read at once from file.
    const size_t BUFFER_BLOCK_SIZE = 100;
    // How many slots to have in the internal queue between read and send.
    const int REPLAY_FASTQUEUE_N_SLOTS = 2;
    // How many frames do we buffer in send.
    const size_t REPLAY_SNDHWM = 100;

    // Writer RECV queue on each ZMQ connection.
    const int WRITER_RCVHWM = 100;
    // ZMQ threads for receiving data from sf_replay.
    const int WRITER_ZMQ_IO_THREADS = 2;
    // MS to retry reading from the image assembler.
    const size_t WRITER_IMAGE_ASSEMBLER_RETRY_MS = 5;
    // How large are metadata chunks in the HDF5.
    const size_t WRITER_METADATA_CHUNK_N_IMAGES = 100;
    // How large should the data cache be in N images.
    const size_t WRITER_DATA_CACHE_N_IMAGES = 100;
}

#endif //BUFFERCONFIG_HPP
