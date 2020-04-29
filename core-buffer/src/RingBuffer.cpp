#include <stdexcept>
#include <iostream>
#include <cstddef>

#include "RingBuffer.hpp"

using namespace std;

template <class T>
RingBuffer<T>::RingBuffer(size_t n_slots) :
        n_slots_(n_slots),
        ringbuffer_slots_(n_slots, 0)
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono; 
        cout << "[" << system_clock::now() << "]";
        cout << "[RingBuffer::RingBuffer]";
        cout << " Creating ring buffer";
        cout << " with n_slots " << n_slots << endl;
    #endif
}

template <class T>
RingBuffer<T>::~RingBuffer()
{
    if (frame_data_buffer_ != NULL) {
        free(frame_data_buffer_);
        frame_data_buffer_ = NULL;
    }
}

template <class T>
void RingBuffer<T>::initialize(const size_t requested_slot_size)
{
    if (is_initialized()) {
        return;
    }

    lock_guard<mutex> lock(ringbuffer_slots_mutex_);

    if (initialized_) {

        if (requested_slot_size > slot_size_) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[RingBuffer::initialize]";
            err_msg << " Already initialized with smaller slot_size ";
            err_msg << slot_size_ << " than requested_slot_size ";
            err_msg << requested_slot_size << endl;

            throw runtime_error(err_msg.str());
        }

        return;
    }

    write_index_ = 0;
    slot_size_ = requested_slot_size;
    buffer_size_ = slot_size_ * n_slots_;
    frame_data_buffer_ = new char[buffer_size_];
    buffer_used_slots_ = 0;

    initialized_ = true;

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[RingBuffer::initialize]";
        cout << " Ringbuffer initialized";
        cout << " with slot_size " << slot_size_;
        cout << " and n_slots " << n_slots_;
        cout << " and buffer_size " << buffer_size_ << endl;
    #endif
}

template <class T>
char* RingBuffer<T>::reserve(shared_ptr<T> frame_metadata)
{
    if (!is_initialized()) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[RingBuffer::reserve]";
        err_msg << " Ringbuffer not initialized.";

        throw runtime_error(err_msg.str());
    }

    if (frame_metadata->frame_bytes_size > slot_size_) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono; 
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[RingBuffer::reserve]";
        err_msg << " Received frame index " << frame_metadata->frame_index;
        err_msg << " that is too large for ring buffer slot.";
        err_msg << " Slot size " << slot_size_ << ", but frame bytes size ";
        err_msg << frame_metadata->frame_bytes_size << endl;

        throw runtime_error(err_msg.str());
    }

    // Check and reserve slot in the buffer.
    {
        lock_guard<mutex> lock(ringbuffer_slots_mutex_);

        if (!ringbuffer_slots_[write_index_]) {
            ringbuffer_slots_[write_index_] = true;
            
            frame_metadata->buffer_slot_index = write_index_;

            #ifdef DEBUG_OUTPUT
                using namespace date;
                using namespace chrono; 
                cout << "[" << system_clock::now() << "]";
                cout << "[RingBuafer::reserve]";
                cout << " Ring buffer slot ";
                cout << frame_metadata->buffer_slot_index;
                cout << " reserved for frame_index ";
                cout << frame_metadata->frame_index << endl;
            #endif

            write_index_ = (write_index_ + 1) % n_slots_;
            buffer_used_slots_++;

        } else {
            return nullptr;
        }
    }

    return get_buffer_slot_address(frame_metadata->buffer_slot_index);
}

template <class T>
void RingBuffer<T>::commit(shared_ptr<T> frame_metadata)
{
    lock_guard<mutex> lock(frame_metadata_queue_mutex_);

    frame_metadata_queue_.push_back(frame_metadata);

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[RingBuffer::commit]";
        cout << " Metadata for frame_index " << frame_metadata->frame_index;
        cout << " added to metadata queue." << endl;
    #endif
}

template <class T>
char* RingBuffer<T>::get_buffer_slot_address(size_t buffer_slot_index)
{
    char* slot_memory_address =
            frame_data_buffer_ + (buffer_slot_index * slot_size_);

    // Check if the memory address is valid.
    if (slot_memory_address > frame_data_buffer_ + buffer_size_) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono; 
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[RingBuffer::get_buffer_slot_address]";
        err_msg << " Ring buffer address out of range." << endl;

        throw runtime_error(err_msg.str());
    }

    return slot_memory_address;
}

template <class T>
pair<shared_ptr<T>, char*> RingBuffer<T>::read()
{
    shared_ptr<T> frame_metadata;

    {
        lock_guard<mutex> lock(frame_metadata_queue_mutex_);

        // A NULL char* means no waiting data in the ring buffer.
        if (frame_metadata_queue_.empty()) {
            return {NULL, NULL};
        }

        frame_metadata = frame_metadata_queue_.front();
        frame_metadata_queue_.pop_front();
    }

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono; 
        cout << "[" << system_clock::now() << "]";
        cout << "[RingBuffer::read]";
        cout << " Received metadata for frame_index ";
        cout << frame_metadata->frame_index << endl;
    #endif

    // Check if the references ring buffer slot is valid.
    {
        lock_guard<mutex> lock(ringbuffer_slots_mutex_);

        if (!ringbuffer_slots_[frame_metadata->buffer_slot_index]) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[RingBuffer::read]";
            err_msg << " Ring buffer slot";
            err_msg << " referenced in message header ";
            err_msg << frame_metadata->buffer_slot_index << " is empty.";
            err_msg << endl;

            throw runtime_error(err_msg.str());
        }
    }
            
    return {frame_metadata,
            get_buffer_slot_address(frame_metadata->buffer_slot_index)};
}

template <class T>
void RingBuffer<T>::release(size_t buffer_slot_index)
{
    if (buffer_slot_index >= n_slots_) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono; 
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[RingBuffer::release]";
        err_msg << " Slot index " << buffer_slot_index << " is out of range.";
        err_msg << " Ring buffer n_slots = " << n_slots_ << endl;

        throw runtime_error(err_msg.str());
    }

    {
        lock_guard<mutex> lock(ringbuffer_slots_mutex_);

        if (ringbuffer_slots_[buffer_slot_index]) {
            ringbuffer_slots_[buffer_slot_index] = false;

            buffer_used_slots_--;

        } else {
            stringstream err_msg;

            using namespace date;
            using namespace chrono; 
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[RingBuffer::release]";
            err_msg << " Cannot release empty ring buffer slot ";
            err_msg << buffer_slot_index << endl;

            throw runtime_error(err_msg.str());
        }
    }
}

template <class T>
bool RingBuffer<T>::is_empty()
{
    lock_guard<mutex> lock(ringbuffer_slots_mutex_);
    
    return buffer_used_slots_ == 0;
}

template <class T>
bool RingBuffer<T>::is_initialized()
{
    return initialized_.load(memory_order_relaxed);
}

template <class T>
void RingBuffer<T>::clear()
{
    lock_guard<mutex> lock_slots(ringbuffer_slots_mutex_);
    lock_guard<mutex> lock_metadata(frame_metadata_queue_mutex_);

    write_index_ = 0;
    buffer_used_slots_ = 0;
    ringbuffer_slots_ = vector<bool>(n_slots_, 0);
    frame_metadata_queue_.clear();
}

template <class T>
size_t RingBuffer<T>::get_slot_size()
{
    return slot_size_;
}

template class RingBuffer<FrameMetadata>;
template class RingBuffer<UdpFrameMetadata>;