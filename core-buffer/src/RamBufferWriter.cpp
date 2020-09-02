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
    size_t buffer_size = slot_size * n_modules * n_slots;

    if ((ftruncate(shm_fd_, buffer_size)) == -1) {
        throw runtime_error(strerror(errno));
    }

    void* buffer = mmap(NULL, buffer_size, PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (buffer ==  (void*)-1) {
        throw runtime_error(strerror(errno));
    }
}

RamBufferWriter::~RamBufferWriter()
{
   shm_unlink(detector_name_.c_str());
}
