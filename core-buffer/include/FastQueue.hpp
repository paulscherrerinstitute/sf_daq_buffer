#ifndef FASTQUEUE_HPP
#define FASTQUEUE_HPP

#include <cstddef>
#include <cstdint>
#include <atomic>

template <class T>
class FastQueue {
    const size_t slot_n_bytes_;
    const size_t n_slots_;
    char* buffer_;
    std::atomic_int* buffer_status_;

    uint16_t write_slot_id_;
    uint16_t read_slot_id_;

public:

    enum SLOT_STATUS {
        EMPTY=0,
        RESERVED=1,
        READY=2
    };

    FastQueue(const size_t slot_data_n_bytes, const uint16_t n_slots);
    virtual ~FastQueue();

    T* get_metadata_buffer(const int slot_id);
    char* get_data_buffer(const int slot_id);

    int reserve();
    void commit();

    int read();
    void release();
};


#endif //FASTQUEUE_HPP
