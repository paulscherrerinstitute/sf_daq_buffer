#include "BufferBinaryReader.hpp"

#include <unistd.h>
#include <sstream>
#include <date.h>
#include <cstring>
#include <fcntl.h>

using namespace std;

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
        const uint64_t block_number, BufferBinaryBlock *buffer)
{

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
