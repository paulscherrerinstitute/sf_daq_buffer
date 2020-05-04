#include <BufferUtils.hpp>
#include "FastH5Writer.hpp"
#include <chrono>
#include <WriterUtils.hpp>
#include <cstring>

extern "C"
{
    #include "H5DOpublic.h"
}

using namespace std;

FastH5Writer::FastH5Writer(
        const size_t n_frames_per_file,
        const uint16_t y_frame_size,
        const uint16_t x_frame_size,
        const string& device_name,
        const string& root_folder) :
            n_frames_per_file_(n_frames_per_file),
            y_frame_size_(y_frame_size),
            x_frame_size_(x_frame_size),
            device_name_(device_name),
            root_folder_(root_folder),
            latest_filename_(root_folder + "/" + device_name + "/LATEST"),
            current_filename_(root_folder + "/" + device_name + "/CURRENT"),
            frame_bytes_size_(2 * y_frame_size * x_frame_size),
            current_output_filename_(""),
            current_output_file_(),
            current_image_dataset_(),
            current_pulse_id_(0)
{
}

void FastH5Writer::create_file(const string& filename)
{
    {
        current_output_file_ = H5::H5File(filename,
                H5F_ACC_TRUNC | H5F_ACC_SWMR_WRITE);

        current_output_filename_ = filename;

        hsize_t dataset_dimension[3] =
                {n_frames_per_file_, y_frame_size_, x_frame_size_};
        hsize_t max_dataset_dimension[3] =
                {n_frames_per_file_, y_frame_size_, x_frame_size_};
        H5::DataSpace dataspace(
                3, dataset_dimension, max_dataset_dimension);

        hsize_t dataset_chunking[3] =
                {CHUNKING_FACTOR, y_frame_size_, x_frame_size_};
        H5::DSetCreatPropList dataset_properties;
        dataset_properties.setChunk(3, dataset_chunking);

        current_output_file_.createDataSet(
                "image",
                H5::PredType::NATIVE_UINT16,
                dataspace,
                dataset_properties);

        for (auto &metadata:scalar_metadata_) {
            auto dataset_name = metadata.first;
            auto dataset_type = metadata.second;

            hsize_t dataset_dimension[2] = {n_frames_per_file_, 1};
            H5::DataSpace dataspace(2, dataset_dimension);
            current_output_file_.createDataSet(
                    dataset_name, dataset_type, dataspace);
        }
    }

    current_output_file_.close();

    current_output_file_ =
            H5::H5File(filename, H5F_ACC_RDWR |H5F_ACC_SWMR_WRITE);

    current_image_dataset_ = current_output_file_.openDataSet("image");

    for (auto& metadata:scalar_metadata_) {
        auto dataset_name = metadata.first;
        auto dataset_type = metadata.second;

        auto dataset = current_output_file_.openDataSet(dataset_name);

        datasets_.insert({dataset_name, dataset});

        size_t n_buffer_bytes =
                dataset.getDataType().getSize() * n_frames_per_file_;
        buffers_.insert(
                {dataset_name, new char[n_buffer_bytes]});
    }

}

FastH5Writer::~FastH5Writer()
{
    close_file();
}

void FastH5Writer::close_file() {
    current_output_filename_ = "";
    current_output_file_.close();
    current_image_dataset_.close();
    current_pulse_id_ = 0;
    current_frame_index_ = 0;

    for (auto &dataset:datasets_) {
        dataset.second.close();
    }
    datasets_.clear();

    for (auto &buffer:buffers_) {
        delete [] buffer.second;
    }
    buffers_.clear();
}

void FastH5Writer::set_pulse_id(const uint64_t pulse_id)
{
    current_pulse_id_ = pulse_id;
    current_frame_index_ = BufferUtils::get_file_frame_index(pulse_id);

    auto new_output_filename = BufferUtils::get_filename(
            root_folder_, device_name_, pulse_id);

    if (new_output_filename != current_output_filename_){

        if (current_output_file_.getId() != -1) {
            auto latest = current_output_filename_;
            close_file();
            BufferUtils::update_latest_file(
                    latest_filename_, latest);
        }

        WriterUtils::create_destination_folder(new_output_filename);
        create_file(new_output_filename);

        BufferUtils::update_latest_file(
                current_filename_, current_output_filename_);
    }
}

void FastH5Writer::write_data(const char *buffer)
{
    hsize_t buff_dim[2] = {y_frame_size_, x_frame_size_};
    H5::DataSpace buffer_space (2, buff_dim);

    hsize_t disk_dim[3] = {n_frames_per_file_, y_frame_size_, x_frame_size_};
    H5::DataSpace disk_space(3, disk_dim);

    hsize_t count[] = {1, y_frame_size_, x_frame_size_};
    hsize_t start[] = {current_frame_index_, 0, 0};
    disk_space.selectHyperslab(H5S_SELECT_SET, count, start);

    current_image_dataset_.write(
            buffer,
            H5::PredType::NATIVE_UINT16,
            buffer_space,
            disk_space);

    H5Dflush(current_image_dataset_.getId());
}

template <>
void FastH5Writer::write_scalar_metadata<uint64_t>(
        const std::string& name,
        const void* value)
{
    write_scalar_metadata(name, value, H5::PredType::NATIVE_UINT64);
}

template <>
void FastH5Writer::write_scalar_metadata<uint32_t>(
        const std::string& name,
        const void* value)
{
    write_scalar_metadata(name, value, H5::PredType::NATIVE_UINT32);
}

template <>
void FastH5Writer::write_scalar_metadata<uint16_t>(
        const std::string& name,
        const void* value)
{
    write_scalar_metadata(name, value, H5::PredType::NATIVE_UINT16);
}

void FastH5Writer::write_scalar_metadata(
        const std::string& name,
        const void* value,
        const H5::DataType data_type)
{
    auto dataset = datasets_.at(name);

    hsize_t buff_dim[1] = {1};
    H5::DataSpace buffer_space (1, buff_dim);

    hsize_t disk_dim[2] = {n_frames_per_file_, 1};
    H5::DataSpace disk_space(1, disk_dim);

    hsize_t count[] = {1, 1};
    hsize_t start[] = {current_frame_index_, 0};
    disk_space.selectHyperslab(H5S_SELECT_SET, count, start);

    dataset.write(
            value,
            data_type,
            buffer_space,
            disk_space);

    H5Dflush(dataset.getId());
}

template <>
void FastH5Writer::add_scalar_metadata<uint64_t>(
        const std::string& metadata_name)
{
    scalar_metadata_.insert({metadata_name, H5::PredType::NATIVE_UINT64});
}

template <>
void FastH5Writer::add_scalar_metadata<uint32_t>(
        const std::string& metadata_name)
{
    scalar_metadata_.insert({metadata_name, H5::PredType::NATIVE_UINT32});
}

template <>
void FastH5Writer::add_scalar_metadata<uint16_t>(
        const std::string& metadata_name)
{
    scalar_metadata_.insert({metadata_name, H5::PredType::NATIVE_UINT16});
}