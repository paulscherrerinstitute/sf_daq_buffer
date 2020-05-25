#include "WriterZmqReceiver.hpp"
#include "zmq.h"
#include "date.h"
#include <chrono>
#include <sstream>

using namespace std;
using namespace core_buffer;

WriterZmqReceiver::WriterZmqReceiver(
        void *ctx,
        const string &ipc_prefix,
        const size_t n_modules) :
            n_modules_(n_modules),
            sockets_(n_modules)
{

    for (size_t i = 0; i < n_modules; i++) {
        sockets_[i] = zmq_socket(ctx, ZMQ_PULL);

        int rcvhwm = WRITER_RCVHWM;
        if (zmq_setsockopt(sockets_[i], ZMQ_RCVHWM, &rcvhwm,
                           sizeof(rcvhwm)) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }
        int linger = 0;
        if (zmq_setsockopt(sockets_[i], ZMQ_LINGER, &linger,
                           sizeof(linger)) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }

        stringstream ipc_addr;
        ipc_addr << ipc_prefix << i;
        const auto ipc = ipc_addr.str();

        if (zmq_connect(sockets_[i], ipc.c_str()) != 0) {
            throw runtime_error(zmq_strerror(errno));
        }
    }
}

WriterZmqReceiver::~WriterZmqReceiver()
{
    for (size_t i = 0; i < n_modules_; i++) {
        zmq_close(sockets_[i]);
    }
}

void WriterZmqReceiver::get_next_buffer(
        const uint64_t pulse_id,
        ImageMetadataBuffer* image_metadata,
        char* image_buffer)
{
    // Init the image metadata.
    image_metadata->pulse_id = pulse_id;
    image_metadata->frame_index = 0;
    image_metadata->daq_rec = 0;
    image_metadata->data_n_bytes = 0;
    image_metadata->is_good_frame = 1;
    bool image_metadata_init = false;

    size_t image_buffer_offset = 0;

    for (size_t i_module = 0; i_module < n_modules_; i_module++) {

        auto n_bytes_metadata = zmq_recv(
                sockets_[i_module],
                &frame_metadata,
                sizeof(ReplayModuleFrameBuffer),
                0);

        if (n_bytes_metadata != sizeof(ReplayModuleFrameBuffer)) {
            throw runtime_error("Wrong number of metadata bytes.");
        }

        for (size_t i=0; i<REPLAY_READ_BUFFER_SIZE; i++) {

        }

        // sf_replay should always send the right pulse_id.
        if (frame_metadata.metadata.pulse_id != pulse_id) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[sf_writer::receive_replay]";
            err_msg << " Read unexpected pulse_id. ";
            err_msg << " Expected " << pulse_id;
            err_msg << " received ";
            err_msg << frame_metadata.metadata.pulse_id;
            err_msg << " from i_module " << i_module << endl;

            throw runtime_error(err_msg.str());
        }

        if (!frame_metadata.is_frame_present) {
            image_metadata->is_good_frame = 0;

        // Init the image metadata with the first valid frame.
        } else if (!image_metadata_init) {
            image_metadata_init = true;

            image_metadata->frame_index =
                    frame_metadata.metadata.frame_index;
            image_metadata->daq_rec =
                    frame_metadata.metadata.daq_rec;
        }

        // Once the image is not good, we don't care to re-flag it.
        if (image_metadata->is_good_frame == 1) {
            if (frame_metadata.metadata.frame_index !=
                image_metadata->frame_index) {
                image_metadata->is_good_frame = 0;
            }

            if (frame_metadata.metadata.daq_rec !=
                image_metadata->daq_rec) {
                image_metadata->is_good_frame = 0;
            }

            if (frame_metadata.metadata.n_received_packets !=
                JUNGFRAU_N_PACKETS_PER_FRAME) {
                image_metadata->is_good_frame = 0;
            }
        }

        auto n_bytes_image = zmq_recv(
                sockets_[i_module],
                (image_buffer + image_buffer_offset),
                frame_metadata.data_n_bytes,
                0);

        if (n_bytes_image != frame_metadata.data_n_bytes) {
            throw runtime_error("Wrong number of data bytes.");
        }

        image_buffer_offset += n_bytes_image;
    }

    image_metadata->data_n_bytes = image_buffer_offset;
}
