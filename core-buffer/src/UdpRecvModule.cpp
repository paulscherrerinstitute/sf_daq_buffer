#include "UdpRecvModule.hpp"
#include "jungfrau.hpp"
#include <iostream>
#include <UdpReceiver.hpp>

using namespace std;

UdpRecvModule::UdpRecvModule(
        FastQueue<ModuleFrame>& queue,
        const uint16_t udp_port) :
            queue_(queue),
            is_receiving_(true)
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[UdpRecvModule::UdpRecvModule]";
        cout << " Starting with ";
        cout << "udp_port " << udp_port << endl;
    #endif

    receiving_thread_ = thread(
            &UdpRecvModule::receive_thread, this,
            udp_port);
}

UdpRecvModule::~UdpRecvModule()
{
    is_receiving_ = false;
    receiving_thread_.join();
}


void UdpRecvModule::receive_thread(const uint16_t udp_port)
{
    try {
        UdpReceiver udp_receiver;
        udp_receiver.bind(udp_port);

        ModuleFrame* module_frame;
        module_frame->pulse_id = 0;
        module_frame->n_received_packets = 0;

        jungfrau_packet packet_buffer;

        auto slot_id = queue_.reserve();

        if (slot_id == -1) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[UdpRecvModule::receive_thread]";
            err_msg << " Queue is full.";
            err_msg << endl;

            throw runtime_error(err_msg.str());
        }

        while (is_receiving_.load(memory_order_relaxed)) {

            if (!udp_receiver.receive(
                    &packet_buffer,
                    JUNGFRAU_BYTES_PER_PACKET)) {
                continue;
            }

            // TODO: Horrible. Breake it down into methods.

            // First packet for this frame.
            if (frame_metadata->pulse_id == 0) {
                frame_metadata->frame_index = packet_buffer.framenum;
                frame_metadata->pulse_id = packet_buffer.bunchid;
                frame_metadata->daq_rec = packet_buffer.debug;
            // Packet from new frame, while we lost the last packet of
            // previous frame.
            } else if (frame_metadata->pulse_id != packet_buffer.bunchid) {
                ring_buffer_.commit(metadata);

                metadata = make_shared<UdpFrameMetadata>();
                metadata->frame_bytes_size = JUNGFRAU_DATA_BYTES_PER_FRAME;
                metadata->pulse_id = 0;
                metadata->n_recv_packets = 0;

                frame_buffer = ring_buffer_.reserve(metadata);
                if (frame_buffer == nullptr) {
                    stringstream err_msg;

                    using namespace date;
                    using namespace chrono;
                    err_msg << "[" << system_clock::now() << "]";
                    err_msg << "[UdpRecvModule::receive_thread]";
                    err_msg << " Ring buffer is full.";
                    err_msg << endl;

                    throw runtime_error(err_msg.str());
                }
                memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);

                frame_metadata->frame_index = packet_buffer.framenum;
                frame_metadata->pulse_id = packet_buffer.bunchid;
                frame_metadata->daq_rec = packet_buffer.debug;
            }

            size_t frame_buffer_offset =
                    JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer.packetnum;

            memcpy(
                    (void*) (frame_buffer + frame_buffer_offset),
                    packet_buffer.data,
                    JUNGFRAU_DATA_BYTES_PER_PACKET);

            frame_metadata->n_recv_packets++;

            // Frame finished with last packet.
            if (packet_buffer.packetnum == JUNGFRAU_N_PACKETS_PER_FRAME-1)
            {
                ring_buffer_.commit(metadata);

                metadata = make_shared<UdpFrameMetadata>();
                metadata->frame_bytes_size = JUNGFRAU_DATA_BYTES_PER_FRAME;
                metadata->pulse_id = 0;
                metadata->n_recv_packets = 0;

                frame_buffer = ring_buffer_.reserve(metadata);
                if (frame_buffer == nullptr) {
                    stringstream err_msg;

                    using namespace date;
                    using namespace chrono;
                    err_msg << "[" << system_clock::now() << "]";
                    err_msg << "[UdpRecvModule::receive_thread]";
                    err_msg << " Ring buffer is full.";
                    err_msg << endl;

                    throw runtime_error(err_msg.str());
                }
                memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);
            }
        }

    } catch (const std::exception& e) {
        is_receiving_ = false;

        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[UdpRecvModule::receive_thread]";
        cout << " Stopped because of exception: " << endl;
        cout << e.what() << endl;

        throw;
    }
}