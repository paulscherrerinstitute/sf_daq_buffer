#include "LiveRecvModule.hpp"

#include <iostream>
#include <date.h>

#include "buffer_config.hpp"
#include "stream_config.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace stream_config;

LiveRecvModule::LiveRecvModule(
        FastQueue<ModuleFrameBuffer>& queue_,
        ZmqLiveReceiver& receiver) :
            queue_(queue_),
            receiver_(receiver),
            is_receiving_(true)
{
    receiving_thread_ = thread(&LiveRecvModule::receive_thread, this);
}

LiveRecvModule::~LiveRecvModule()
{
    stop();
}

void LiveRecvModule::stop()
{
    is_receiving_ = false;
    receiving_thread_.join();
}

void LiveRecvModule::receive_thread()
{
    try {

        int slot_id;
        while(is_receiving_.load(memory_order_relaxed)) {

            while ((slot_id == queue_.reserve()) == -1) {
                this_thread::sleep_for(milliseconds(RB_READ_RETRY_INTERVAL_MS));
            }

            auto meta = queue_.get_metadata_buffer(slot_id);
            auto data = queue_.get_data_buffer(slot_id);

            receiver_.get_next_image(meta, data);

            queue_.commit();
        }

    } catch (const std::exception& e) {
        is_receiving_ = false;

        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[LiveRecvModule::receive_thread]";
        cout << " Stopped because of exception: " << endl;
        cout << e.what() << endl;

        throw;
    }
}