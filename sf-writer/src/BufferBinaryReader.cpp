#include "BufferBinaryReader.hpp"

#include <unistd.h>
#include <sstream>
#include <date.h>
#include <cstring>
#include <fcntl.h>

#include "BufferUtils.hpp"

using namespace std;
using namespace core_buffer;

BufferBinaryReader::BufferBinaryReader(
        const std::string &device,
        const std::string &channel_name) :
            device_(device),
            channel_name_(channel_name),
            current_input_file_(""),
            input_file_fd_(-1)
{

}

BufferBinaryReader::~BufferBinaryReader()
{
    close_current_file();
}

void BufferBinaryReader::get_block(
        const uint64_t block_id, BufferBinaryBlock* buffer)
{
    uint64_t block_start_pulse_id = block_id * BUFFER_BLOCK_SIZE;
    auto current_block_file = BufferUtils::get_filename(
            device_, channel_name_, block_start_pulse_id);

    if (current_block_file != current_input_file_)  {
        open_file(current_block_file);
    }

    size_t file_start_index =
            BufferUtils::get_file_frame_index(block_start_pulse_id);
    size_t n_bytes_offset = file_start_index * sizeof(BufferBinaryFormat);

    auto lseek_result = lseek(input_file_fd_, n_bytes_offset, SEEK_SET);
    if (lseek_result < 0) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[BufferBinaryReader::get_block]";
        err_msg << " Error while lseek on file ";
        err_msg << current_input_file_;
        err_msg << " for n_bytes_offset ";
        err_msg << n_bytes_offset << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    auto n_bytes = ::read(input_file_fd_, buffer,
            sizeof(BufferBinaryFormat) * BUFFER_BLOCK_SIZE);

    if (n_bytes < sizeof(BufferBinaryFormat)) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[BufferBinaryReader::get_block]";
        err_msg << " Error while reading from file ";
        err_msg << current_input_file_ << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }
}

void BufferBinaryReader::open_file(const std::string& filename)
{
    close_current_file();

    input_file_fd_ = open(filename.c_str(), O_RDONLY);

    if (input_file_fd_ < 0) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[BufferBinaryReader::open_file]";
        err_msg << " Cannot open file ";
        err_msg << filename << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    current_input_file_ = filename;
}

void BufferBinaryReader::close_current_file()
{
    if (input_file_fd_ != -1) {
        if (close(input_file_fd_) < 0) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[BinaryWriter::close_current_file]";
            err_msg << " Error while closing file ";
            err_msg << current_input_file_ << ": ";
            err_msg << strerror(errno) << endl;

            throw runtime_error(err_msg.str());
        }

        input_file_fd_ = -1;
        current_input_file_ = "";
    }
}
