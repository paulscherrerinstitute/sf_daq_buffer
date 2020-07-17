#include "BufferBinaryWriter.hpp"

#include <unistd.h>
#include <iostream>
#include "date.h"
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>

#include "BufferUtils.hpp"

using namespace std;

BufferBinaryWriter::BufferBinaryWriter(
        const string& root_folder,
        const string& device_name):
        root_folder_(root_folder),
        device_name_(device_name),
        latest_filename_(root_folder + "/" + device_name + "/LATEST"),
        current_output_filename_(""),
        output_file_fd_(-1)
{
}

BufferBinaryWriter::~BufferBinaryWriter()
{
    close_current_file();
}

void BufferBinaryWriter::write(
        uint64_t pulse_id,
        const BufferBinaryFormat* buffer)
{
    auto current_frame_file =
            BufferUtils::get_filename(root_folder_, device_name_, pulse_id);

    if (current_frame_file != current_output_filename_) {
        open_file(current_frame_file);
    }

    size_t n_bytes_offset =
            BufferUtils::get_file_frame_index(pulse_id) *
            sizeof(BufferBinaryFormat);

    auto lseek_result = lseek(output_file_fd_, n_bytes_offset, SEEK_SET);
    if (lseek_result < 0) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[BufferBinaryWriter::write]";
        err_msg << " Error while lseek on file ";
        err_msg << current_output_filename_;
        err_msg << " for n_bytes_offset ";
        err_msg << n_bytes_offset << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    auto n_bytes = ::write(output_file_fd_, buffer, sizeof(BufferBinaryFormat));
    if (n_bytes < sizeof(BufferBinaryFormat)) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[BufferBinaryWriter::write]";
        err_msg << " Error while writing to file ";
        err_msg << current_output_filename_ << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }
}

void BufferBinaryWriter::open_file(const std::string& filename)
{
    close_current_file();

    BufferUtils::create_destination_folder(filename);

    output_file_fd_ = ::open(filename.c_str(), O_WRONLY | O_CREAT,
                             S_IRWXU | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (output_file_fd_ < 0) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[BinaryWriter::open_file]";
        err_msg << " Cannot create file ";
        err_msg << filename << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    // TODO: Remove context if test successful.

    /** Setting the buffer file size in advance to try to lower the number of
        metadata updates on GPFS. */
    {
        // TODO: Try instead to use fallocate.
        if (lseek(output_file_fd_, MAX_FILE_BYTES, SEEK_SET) < 0) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[BufferBinaryWriter::open_file]";
            err_msg << " Error while lseek on end of file ";
            err_msg << current_output_filename_;
            err_msg << " for MAX_FILE_BYTES ";
            err_msg << MAX_FILE_BYTES << ": ";
            err_msg << strerror(errno) << endl;

            throw runtime_error(err_msg.str());
        }

        const uint8_t mark = 255;
        if(::write(output_file_fd_, &mark, sizeof(mark)) != sizeof(mark)) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[BufferBinaryWriter::open_file]";
            err_msg << " Error while writing to file ";
            err_msg << current_output_filename_ << ": ";
            err_msg << strerror(errno) << endl;

            throw runtime_error(err_msg.str());
        }
    }


    current_output_filename_ = filename;
}

void BufferBinaryWriter::close_current_file()
{
    if (output_file_fd_ != -1) {
        if (close(output_file_fd_) < 0) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[BufferBinaryWriter::close_current_file]";
            err_msg << " Error while closing file ";
            err_msg << current_output_filename_ << ": ";
            err_msg << strerror(errno) << endl;

            throw runtime_error(err_msg.str());
        }

        output_file_fd_ = -1;

        BufferUtils::update_latest_file(
                latest_filename_, current_output_filename_);

        current_output_filename_ = "";
    }
}