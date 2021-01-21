#include "ImageBinaryWriter.hpp"

#include <unistd.h>
#include <iostream>
#include "date.h"
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>

#include "BufferUtils.hpp"

using namespace std;
using namespace buffer_config;

ImageBinaryWriter::ImageBinaryWriter(
        const string& detector_folder,
        const size_t image_n_bytes):
        IMAGE_BYTES(image_n_bytes),
        IMAGE_SLOT_BYTES(IMAGE_BYTES + sizeof(ImageMetadata)),
        MAX_FILE_BYTES(IMAGE_SLOT_BYTES * FILE_MOD),
        detector_folder_(detector_folder),
        latest_filename_(detector_folder + "/LATEST"),
        current_output_filename_(""),
        output_file_fd_(-1)
{
}

ImageBinaryWriter::~ImageBinaryWriter()
{
    close_current_file();
}

void ImageBinaryWriter::write(const ImageMetadata meta, const char* data)
{
    auto current_frame_file =
            BufferUtils::get_image_filename(detector_folder_, meta.pulse_id);

    if (current_frame_file != current_output_filename_) {
        open_file(current_frame_file);
    }

    size_t n_bytes_offset =
            BufferUtils::get_file_frame_index(meta.pulse_id) * IMAGE_SLOT_BYTES;

    auto lseek_result = lseek(output_file_fd_, n_bytes_offset, SEEK_SET);
    if (lseek_result < 0) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[ImageBinaryWriter::write]";
        err_msg << " Error while lseek on file ";
        err_msg << current_output_filename_;
        err_msg << " for n_bytes_offset ";
        err_msg << n_bytes_offset << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    auto n_bytes_meta = ::write(output_file_fd_, &meta, sizeof(ImageMetadata));
    if (n_bytes_meta < sizeof(ImageMetadata)) {
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

    auto n_bytes_data = ::write(output_file_fd_, data, IMAGE_BYTES);
    if (n_bytes_data < sizeof(IMAGE_BYTES)) {
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

void ImageBinaryWriter::open_file(const std::string& filename)
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
        err_msg << "[ImageBinaryWriter::open_file]";
        err_msg << " Cannot create file ";
        err_msg << filename << ": ";
        err_msg << strerror(errno) << endl;

        throw runtime_error(err_msg.str());
    }

    current_output_filename_ = filename;
}

void ImageBinaryWriter::close_current_file()
{
    if (output_file_fd_ != -1) {
        if (close(output_file_fd_) < 0) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[ImageBinaryWriter::close_current_file]";
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