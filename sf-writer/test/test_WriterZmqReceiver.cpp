#include <gtest/gtest.h>
#include "WriterZmqReceiver.hpp"
#include "bitshuffle/bitshuffle.h"
#include <thread>
#include <sstream>
#include "buffer_config.hpp"
#include "zmq.h"

using namespace std;
using namespace core_buffer;

TEST(WriterZmqReceiver, basic_test)
{
    size_t n_modules = 4;
    uint64_t pulse_id = 12345;

    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, 1);

    void* sockets[n_modules];
    for (size_t i = 0; i < n_modules; i++) {
        sockets[i] = zmq_socket(ctx, ZMQ_PUSH);

        int linger = 0;
        if (zmq_setsockopt(sockets[i], ZMQ_LINGER, &linger,
                           sizeof(linger)) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }

        stringstream ipc_addr;
        ipc_addr << REPLAY_STREAM_IPC_URL << i;
        const auto ipc = ipc_addr.str();

        if (zmq_bind(sockets[i], ipc.c_str()) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }
    }
    this_thread::sleep_for(chrono::milliseconds(100));

    WriterZmqReceiver receiver(ctx, REPLAY_STREAM_IPC_URL, n_modules);
    this_thread::sleep_for(chrono::milliseconds(100));

    size_t compressed_frame_size = 5000;
    auto frame_buffer = make_unique<char[]>(compressed_frame_size);

    ImageMetadata image_metadata;
    auto compress_size = bshuf_compress_lz4_bound(
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);
    auto image_buffer = make_unique<char[]>(compress_size * n_modules);

    for (size_t i = 0; i < n_modules; i++) {

        ReplayModuleFrameBuffer frame_metadata;
        frame_metadata.metadata.pulse_id = pulse_id;
        frame_metadata.metadata.frame_index = pulse_id + 100;
        frame_metadata.metadata.n_received_packets = 128;
        frame_metadata.metadata.daq_rec = 4;

        frame_metadata.is_frame_present = 1;
        frame_metadata.data_n_bytes = compressed_frame_size;

        zmq_send(sockets[i],
                 &frame_metadata,
                 sizeof(ReplayModuleFrameBuffer),
                 ZMQ_SNDMORE);

        zmq_send(sockets[i],
                 (char*)(frame_buffer.get()),
                 compressed_frame_size,
                 0);
    }

    receiver.get_next_batch(pulse_id, &image_metadata, image_buffer.get());
    EXPECT_EQ(pulse_id, image_metadata.pulse_id);
    EXPECT_EQ(image_metadata.is_good_frame, 1);
    EXPECT_EQ(image_metadata.daq_rec, 4);
    EXPECT_EQ(image_metadata.data_n_bytes,
            5000*n_modules);
//            5000*n_modules+BSHUF_LZ4_HEADER_BYTES);
}