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
    receiving_thread_ = thread(
            &UdpRecvModule::receive_thread, this,
            udp_port);
}

UdpRecvModule::~UdpRecvModule()
{
    is_receiving_ = false;
    receiving_thread_.join();
}

inline void UdpRecvModule::init_frame (
        ModuleFrame* frame_metadata,
        jungfrau_packet& packet_buffer)
{
    frame_metadata->pulse_id = packet_buffer.bunchid;
    frame_metadata->frame_index = packet_buffer.framenum;
    frame_metadata->daq_rec = (uint64_t)packet_buffer.debug;
}

inline void UdpRecvModule::reserve_next_frame_buffers(
        ModuleFrame*& frame_metadata,
        char*& frame_buffer)
{
    int slot_id;
    if ((slot_id = queue_.reserve()) == -1)
        throw runtime_error("Queue is full.");

    frame_metadata = queue_.get_metadata_buffer(slot_id);
    frame_metadata->pulse_id = 0;
    frame_metadata->n_received_packets = 0;

    frame_buffer = queue_.get_data_buffer(slot_id);
    memset(frame_buffer, 0, JUNGFRAU_DATA_BYTES_PER_FRAME);
}

void UdpRecvModule::receive_thread(const uint16_t udp_port)
{
    try {

        UdpReceiver udp_receiver;
        udp_receiver.bind(udp_port);

        ModuleFrame* frame_metadata;
        char* frame_buffer;
        reserve_next_frame_buffers(frame_metadata, frame_buffer);

        jungfrau_packet packet_buffer;

        while (is_receiving_.load(memory_order_relaxed)) {

            if (!udp_receiver.receive(
                    &packet_buffer,
                    JUNGFRAU_BYTES_PER_PACKET)) {
                continue;
            }

            // First packet for this frame.
            if (frame_metadata->pulse_id == 0) {
                init_frame(frame_metadata, packet_buffer);

            // Happens if the last packet from the previous frame gets lost.
            } else if (frame_metadata->pulse_id != packet_buffer.bunchid) {
                queue_.commit();
                reserve_next_frame_buffers(frame_metadata, frame_buffer);

                init_frame(frame_metadata, packet_buffer);
            }

            size_t frame_buffer_offset =
                    JUNGFRAU_DATA_BYTES_PER_PACKET * packet_buffer.packetnum;

            memcpy(
                    (void*) (frame_buffer + frame_buffer_offset),
                    packet_buffer.data,
                    JUNGFRAU_DATA_BYTES_PER_PACKET);

            frame_metadata->n_received_packets++;

            // Last frame packet received. Frame finished.
            if (packet_buffer.packetnum == JUNGFRAU_N_PACKETS_PER_FRAME-1)
            {
                queue_.commit();
                reserve_next_frame_buffers(frame_metadata, frame_buffer);
                this_thread::yield();
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