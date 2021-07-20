#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <unistd.h>
#include "RamBuffer.hpp"
#include "date.h"
#include <chrono>
#include <iostream>


using namespace std;
using namespace buffer_config;

RamBuffer::RamBuffer(const string& buffer_name,
                     const size_t meta_n_bytes,
                     const size_t data_n_bytes,
                     const int n_modules,
                     const int n_slots) :
        buffer_name_(buffer_name),
        n_modules_(n_modules),
        n_slots_(n_slots),
        meta_bytes_(meta_n_bytes),
        data_bytes_(data_n_bytes),
        slot_bytes_((meta_bytes_ + data_bytes_) * n_modules),
        buffer_bytes_(slot_bytes_ * n_slots_)
{
#ifdef DEBUG_OUTPUT
    using namespace date;
    cout << " [" << std::chrono::system_clock::now();
    cout << "] [RamBuffer::RamBuffer] ";
    cout << " buffer_name_ " << buffer_name;
    cout << " || n_modules_ " << n_modules_;
    cout << " || n_slots_ " << n_slots_;
    cout << " || meta_bytes_" << meta_bytes_;
    cout << " || data_bytes_" << data_bytes_;
    cout << endl;
#endif

    shm_fd_ = shm_open(buffer_name_.c_str(), O_RDWR | O_CREAT, 0777);
    if (shm_fd_ < 0) {
        throw runtime_error(string("shm_open failed: ") + strerror(errno));
    }

    if ((ftruncate(shm_fd_, buffer_bytes_)) == -1) {
        throw runtime_error(strerror(errno));
    }

    // TODO: Test with MAP_HUGETLB
    buffer_ = static_cast<char *>(mmap(NULL, buffer_bytes_, PROT_WRITE,
                                       MAP_SHARED, shm_fd_, 0));
    if (buffer_ == MAP_FAILED) {
        throw runtime_error(strerror(errno));
    }
}

RamBuffer::~RamBuffer()
{
    munmap(buffer_, buffer_bytes_);
    close(shm_fd_);
    shm_unlink(buffer_name_.c_str());
}

char* RamBuffer::_get_meta_buffer(
        int slot_n,
        uint64_t module_id) const
{   
    return buffer_ + (slot_n * slot_bytes_) +
          (module_id * meta_bytes_);
}

char* RamBuffer::_get_frame_data_buffer(
        int slot_n,
        uint64_t module_id) const
{
    return buffer_ + (slot_n * slot_bytes_) +
           (n_modules_ * meta_bytes_) + (module_id * data_bytes_);
}

void RamBuffer::write_frame(
        const ModuleFrame& src_meta,
        const char *src_data) const
{
    auto *dst_meta = (ModuleFrame*) get_frame_meta(
            src_meta.id, src_meta.module_id);
    auto *dst_data = get_frame_data(
            src_meta.id, src_meta.module_id);

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [RamBuffer::write_frame] ";
        cout << " src_meta.id " << src_meta.id;
        cout << " || src_meta.frame_index " << src_meta.frame_index;
        cout << " || src_meta.n_recv_packets " << src_meta.n_recv_packets;
        cout << " || src_meta.daq_rec " << src_meta.daq_rec;
        cout << " || src_meta.module_id " << src_meta.module_id;
        cout << endl;
    #endif

    memcpy(dst_meta, &src_meta, sizeof(ModuleFrame));
    memcpy(dst_data, src_data, data_bytes_);
}

char* RamBuffer::get_frame_meta(
        const uint64_t image_id,
        const uint64_t module_id) const
{
    const size_t slot_n = image_id % n_slots_;
    return _get_meta_buffer(slot_n, module_id);
}

char* RamBuffer::get_slot_meta(const uint64_t image_id) const
{
    return get_frame_meta(image_id, 0);
}

char* RamBuffer::get_frame_data(
        const uint64_t image_id,
        const uint64_t module_id) const
{
    const size_t slot_n = image_id % n_slots_;
    return _get_frame_data_buffer(slot_n, module_id);
}

char* RamBuffer::get_slot_data(const uint64_t image_id) const
{
    return get_frame_data(image_id, 0);
}