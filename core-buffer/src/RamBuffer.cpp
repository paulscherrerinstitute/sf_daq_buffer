#include <sys/mman.h>
#include <bits/fcntl-linux.h>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include "RamBuffer.hpp"
#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

RamBuffer::RamBuffer(
        const string &detector_name,
        const size_t n_modules,
        const int module_n,
        const size_t n_slots) :
        detector_name_(detector_name),
        module_n_(module_n),
        n_slots_(n_slots),
        image_size_(MODULE_N_BYTES * n_modules),
        buffer_size_((sizeof(ModuleFrame) + image_size_) * n_slots)
{
    shm_fd_ = shm_open(detector_name_.c_str(), O_RDWR | O_CREAT, 0777);
    if (shm_fd_ < 0) {
        throw runtime_error(strerror(errno));
    }

    if ((ftruncate(shm_fd_, buffer_size_)) == -1) {
        throw runtime_error(strerror(errno));
    }

    // TODO: Test with MAP_HUGETLB
    buffer_ = mmap(NULL, buffer_size_, PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (buffer_ == MAP_FAILED) {
        throw runtime_error(strerror(errno));
    }

    meta_buffer_ = (ModuleFrame *) buffer_;
    // Image buffer start right after metadata buffer.
    image_buffer_ = (char *) (meta_buffer_ + n_slots);
}

RamBuffer::~RamBuffer()
{
    munmap(buffer_, buffer_size_);
    close(shm_fd_);
    shm_unlink(detector_name_.c_str());

}

void RamBuffer::write_frame(
        const ModuleFrame &metadata,
        const char *data)
{
    size_t slot_n = metadata.pulse_id % n_slots_;

    ModuleFrame *meta = meta_buffer_ + slot_n;
    *meta = metadata;

    char *frame = image_buffer_ +
                  (image_size_ * slot_n) +
                  (MODULE_N_BYTES * module_n_);

    memcpy(frame, data, MODULE_N_BYTES);
}

