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
        const size_t n_modules,
        const uint64_t stop_pulse_id_) :
        n_modules_(n_modules),
        sockets_(n_modules),
        end_pulse_id_(stop_pulse_id_)
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
        const uint64_t start_pulse_id,
        ImageMetadataBuffer* i_meta,
        char* image_buffer)
{
    auto n_images_in_buffer = WRITER_DATA_CACHE_N_IMAGES;
    auto images_left = end_pulse_id_ - start_pulse_id + 1;
    if (images_left < n_images_in_buffer) {
        n_images_in_buffer = images_left;
    }

    i_meta->n_images = (uint16_t)n_images_in_buffer;

    for (uint64_t i_pulse=0; i_pulse<n_images_in_buffer; i_pulse++) {

        auto pulse_id = start_pulse_id + i_pulse;
        bool pulse_id_initialized = false;

        i_meta->pulse_id[i_pulse] = pulse_id;
        i_meta->is_good_image[i_pulse] = 1;
        i_meta->frame_index[i_pulse] = 0;
        i_meta->daq_rec[i_pulse] = 0;

        for (size_t i_module = 0; i_module < n_modules_; i_module++) {

            auto n_bytes_metadata = zmq_recv(
                    sockets_[i_module], &f_meta_, sizeof(f_meta_), 0);

            if (n_bytes_metadata != sizeof(f_meta_)) {
                throw runtime_error("Wrong number of metadata bytes.");
            }

            if (f_meta_.pulse_id == 0) {
                i_meta->is_good_image[i_pulse] = 0;

            } else {
                if (!pulse_id_initialized) {
                    // Init the image metadata with the first valid frame.
                    pulse_id_initialized = true;

                    i_meta->frame_index[i_pulse] = f_meta_.frame_index;
                    i_meta->daq_rec[i_pulse] = f_meta_.daq_rec;
                }

                if (f_meta_.pulse_id != i_meta->pulse_id[i_pulse]) {
                    stringstream err_msg;

                    err_msg << "[WriterZmqReceiver::get_next_buffer]";
                    err_msg << " Read unexpected pulse_id. ";
                    err_msg << " Expected " << pulse_id;
                    err_msg << " received ";
                    err_msg << f_meta_.pulse_id;
                    err_msg << " from i_module " << i_module << endl;

                    throw runtime_error(err_msg.str());
                }
            }

            // Once the image is not good, we don't care to re-flag it.
            if (i_meta->is_good_image[i_pulse] == 1) {

                if (f_meta_.frame_index != i_meta->frame_index[i_pulse]) {
                    i_meta->is_good_image[i_pulse] = 0;
                }

                if (f_meta_.daq_rec != i_meta->daq_rec[i_pulse]) {
                    i_meta->is_good_image[i_pulse] = 0;
                }

                if (f_meta_.n_received_packets != JF_N_PACKETS_PER_FRAME) {
                    i_meta->is_good_image[i_pulse] = 0;
                }
            }

            auto pulse_offset = i_pulse * n_modules_ * MODULE_N_BYTES ;
            auto module_offset = i_module * MODULE_N_BYTES;

            auto n_bytes_image = zmq_recv(
                    sockets_[i_module],
                    (image_buffer + pulse_offset + module_offset),
                    MODULE_N_BYTES, 0);

            if (n_bytes_image != MODULE_N_BYTES) {
                throw runtime_error("Wrong number of data bytes.");
            }
        }
    }
}
