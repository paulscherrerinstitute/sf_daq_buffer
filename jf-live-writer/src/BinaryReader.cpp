#include "BinaryReader.hpp"

#include <unistd.h>
#include <sstream>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>

#include "BufferUtils.hpp"
#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

BinaryReader::BinaryReader(
        const std::string &detector_folder,
        const std::string &module_name) :
        detector_folder_(detector_folder),
        module_name_(module_name),
        current_input_file_(""),
        input_file_fd_(-1)
{}

BinaryReader::~BinaryReader()
{
    close_current_file();
}

void BinaryReader::get_frame(
        const uint64_t pulse_id, BufferBinaryFormat* buffer)
{

    auto current_frame_file = BufferUtils::get_filename(
            detector_folder_, module_name_, pulse_id);

    if (current_frame_file != current_input_file_)  {
        open_file(current_frame_file);
    }

    size_t file_index = BufferUtils::get_file_frame_index(pulse_id);
    size_t n_bytes_offset = file_index * sizeof(BufferBinaryFormat);

    auto lseek_result = lseek(input_file_fd_, n_bytes_offset, SEEK_SET);
    if (lseek_result < 0) {
        stringstream err_msg;

        err_msg << "[BinaryReader::get_frame]";
        err_msg << " Error while lseek on file ";
        err_msg << current_input_file_ << " for n_bytes_offset ";
        err_msg << n_bytes_offset << ": " << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    auto n_bytes = ::read(input_file_fd_, buffer, sizeof(BufferBinaryFormat));

    if (n_bytes < sizeof(BufferBinaryFormat)) {
        stringstream err_msg;

        err_msg << "[BinaryReader::get_block]";
        err_msg << " Error while reading from file ";
        err_msg << current_input_file_ << ": " << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }
}

void BinaryReader::open_file(const std::string& filename)
{
    close_current_file();

    input_file_fd_ = open(filename.c_str(), O_RDONLY);

    if (input_file_fd_ < 0) {
        stringstream err_msg;

        err_msg << "[BinaryReader::open_file]";
        err_msg << " Cannot open file " << filename << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    current_input_file_ = filename;
}

void BinaryReader::close_current_file()
{
    if (input_file_fd_ != -1) {
        if (close(input_file_fd_) < 0) {
            stringstream err_msg;

            err_msg << "[BinaryWriter::close_current_file]";
            err_msg << " Error while closing file " << current_input_file_;
            err_msg << ": " << strerror(errno) << endl;

            throw runtime_error(err_msg.str());
        }

        input_file_fd_ = -1;
        current_input_file_ = "";
    }
}
