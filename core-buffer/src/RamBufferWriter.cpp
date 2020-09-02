#include <sys/mman.h>
#include <bits/fcntl-linux.h>
#include <cstring>
#include <stdexcept>
#include <formats.hpp>
#include <unistd.h>
#include "RamBufferWriter.hpp"
#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

RamBufferWriter::RamBufferWriter(
        const string &detector_name,
        const int module_n,
        const size_t n_modules,
        const size_t n_slots) :
            detector_name_(detector_name),
            module_n_(module_n)

{
    shm_fd_ = shm_open(detector_name_.c_str(), O_RDWR | O_CREAT, 0777);
    if (shm_fd_ < 0) {
        throw runtime_error(strerror(errno));
    }

    size_t slot_size = MODULE_N_BYTES + sizeof(ModuleFrame);
    buffer_size_ = slot_size * n_modules * n_slots;

    if ((ftruncate(shm_fd_, buffer_size_)) == -1) {
        throw runtime_error(strerror(errno));
    }

    // TODO: Test with MAP_HUGETLB
    void* ptr = mmap(NULL, buffer_size_, PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (ptr ==  MAP_FAILED) {
        throw runtime_error(strerror(errno));
    }

    buffer_ = ptr;
}

RamBufferWriter::~RamBufferWriter()
{
    munmap(buffer_, buffer_size_);
    shm_unlink(detector_name_.c_str());
    close(shm_fd_);
}
