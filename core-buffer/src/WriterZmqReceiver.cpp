//#include <string>
//#include "WriterZmqReceiver.hpp"
//
//void connect()
//{
//    void *sockets[n_modules];
//    for (size_t i = 0; i < n_modules; i++) {
//        sockets[i] = zmq_socket(ctx, ZMQ_PULL);
//
//        int rcvhwm = WRITER_RCVHWM;
//        if (zmq_setsockopt(sockets[i], ZMQ_RCVHWM, &rcvhwm,
//                           sizeof(rcvhwm)) != 0) {
//            throw runtime_error(strerror(errno));
//        }
//        int linger = 0;
//        if (zmq_setsockopt(sockets[i], ZMQ_LINGER, &linger,
//                           sizeof(linger)) != 0) {
//            throw runtime_error(strerror(errno));
//        }
//
//        stringstream ipc_addr;
//        ipc_addr << ipc_prefix << i;
//        const auto ipc = ipc_addr.str();
//
//        if (zmq_bind(sockets[i], ipc.c_str()) != 0) {
//            throw runtime_error(strerror(errno));
//        }
//    }
//}
//
//void disconnect()
//{
//    for (size_t i = 0; i < n_modules; i++) {
//        zmq_close(sockets[i]);
//    }
//}
//
//void acquire_pulse()
//{
//    frame_meta_buffer->is_good_frame[i_buffer] = true;
//
//    for (size_t i_module = 0; i_module < n_modules; i_module++) {
//        auto n_bytes_metadata = zmq_recv(
//                sockets[i_module],
//                module_meta_buffer.get(),
//                sizeof(ModuleFrame),
//                0);
//
//        if (n_bytes_metadata != sizeof(ModuleFrame)) {
//            throw runtime_error("Wrong number of metadata bytes.");
//        }
//
//        if (module_meta_buffer->pulse_id != current_pulse_id) {
//            stringstream err_msg;
//
//            using namespace date;
//            using namespace chrono;
//            err_msg << "[" << system_clock::now() << "]";
//            err_msg << "[sf_writer::receive_replay]";
//            err_msg << " Read unexpected pulse_id. ";
//            err_msg << " Expected " << current_pulse_id;
//            err_msg << " received ";
//            err_msg << module_meta_buffer->pulse_id << endl;
//
//            throw runtime_error(err_msg.str());
//        }
//
//        // Initialize buffers in first iteration for each pulse_id.
//        if (i_module == 0) {
//            frame_meta_buffer->pulse_id[i_buffer] =
//                    module_meta_buffer->pulse_id;
//            frame_meta_buffer->frame_index[i_buffer] =
//                    module_meta_buffer->frame_index;
//            frame_meta_buffer->daq_rec[i_buffer] =
//                    module_meta_buffer->daq_rec;
//            frame_meta_buffer->n_received_packets[i_buffer] =
//                    module_meta_buffer->n_received_packets;
//
//            if ( module_meta_buffer->n_received_packets != 128 ) frame_meta_buffer->is_good_frame[i_buffer] = false;
//
//        } else {
//            if (module_meta_buffer->pulse_id != frame_meta_buffer->pulse_id[i_buffer]) frame_meta_buffer->is_good_frame[i_buffer] = false;
//
//            if (module_meta_buffer->frame_index != frame_meta_buffer->frame_index[i_buffer]) frame_meta_buffer->is_good_frame[i_buffer] = false;
//
//            if (module_meta_buffer->daq_rec != frame_meta_buffer->daq_rec[i_buffer]) frame_meta_buffer->is_good_frame[i_buffer] = false;
//
//            if (module_meta_buffer->n_received_packets != 128 ) frame_meta_buffer->is_good_frame[i_buffer] = false;
//        }
//
//        if (frame_meta_buffer->pulse_id[i_buffer] !=
//            module_meta_buffer->pulse_id) {
//            throw runtime_error("Unexpected pulse_id received.");
//        }
//
//        // Offset due to frame in buffer.
//        size_t offset = MODULE_N_BYTES * n_modules * i_buffer;
//        // offset due to module in frame.
//        offset += MODULE_N_BYTES * i_module;
//
//        auto n_bytes_image = zmq_recv(
//                sockets[i_module],
//                (frame_buffer + offset),
//                MODULE_N_BYTES,
//                0);
//
//        if (n_bytes_image != MODULE_N_BYTES) {
//            throw runtime_error("Wrong number of data bytes.");
//        }
//    }
//}
//
//WriterZmqReceiver::WriterZmqReceiver(void *ctx, const std::string ipc_prefix,
//                                     const size_t n_modules) {
//
//}
