#include "WriterH5Writer.hpp"
#include <sstream>
#include "date.h"

extern "C"
{
    #include "H5DOpublic.h"
}

using namespace std;
using namespace core_buffer;

WriterH5Writer::WriterH5Writer(
        const string& output_file,
        const size_t n_frames,
        const size_t n_modules) :
        n_frames_(n_frames),
        n_modules_(n_modules),
        current_write_index_(0)
{
    file_ = H5::H5File(output_file, H5F_ACC_TRUNC);

    hsize_t image_dataset_dims[3] =
            {n_frames_, n_modules * MODULE_Y_SIZE, MODULE_X_SIZE};

    H5::DataSpace image_dataspace(3, image_dataset_dims);

    hsize_t image_dataset_chunking[3] =
            {1, n_modules * MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DSetCreatPropList image_dataset_properties;
    image_dataset_properties.setChunk(3, image_dataset_chunking);

    image_dataset_ = file_.createDataSet(
            "image",
            H5::PredType::NATIVE_UINT16,
            image_dataspace,
            image_dataset_properties);

    hsize_t metadata_dataset_dims[] = {n_frames_, 1};
    H5::DataSpace metadata_dataspace(2, metadata_dataset_dims);

    hsize_t metadata_dataset_chunking[] = {1, 1};
    H5::DSetCreatPropList metadata_dataset_properties;
    metadata_dataset_properties.setChunk(2, metadata_dataset_chunking);

    pulse_id_dataset_ = file_.createDataSet(
            "pulse_id",
            H5::PredType::NATIVE_UINT64,
            metadata_dataspace,
            metadata_dataset_properties);

    frame_index_dataset_ = file_.createDataSet(
            "frame_index",
            H5::PredType::NATIVE_UINT64,
            metadata_dataspace,
            metadata_dataset_properties);

    daq_rec_dataset_ = file_.createDataSet(
            "daq_rec",
            H5::PredType::NATIVE_UINT32,
            metadata_dataspace,
            metadata_dataset_properties);

    n_received_packets_dataset_ = file_.createDataSet(
            "n_received_packets",
            H5::PredType::NATIVE_UINT16,
            metadata_dataspace,
            metadata_dataset_properties);
}

WriterH5Writer::~WriterH5Writer()
{
    close_file();
}

void WriterH5Writer::close_file()
{
    image_dataset_.close();
    pulse_id_dataset_.close();
    frame_index_dataset_.close();
    daq_rec_dataset_.close();
    n_received_packets_dataset_.close();

    file_.close();
}

void WriterH5Writer::write(const ImageMetadata* metadata, const char* data) {
    auto pulse_id_data = (char*)(metadata->pulse_id);
    auto frame_index_data = (char*)(metadata->frame_index);
    auto daq_rec_data = (char*)(metadata->daq_rec);
    auto n_received_packets_data = (char*)(metadata->n_received_packets);
    auto is_good_frame_data = (char*)(metadata->is_good_frame);

    hsize_t image_offset[] = {current_write_index_, 0, 0};
    hsize_t metadata_offset [] = {current_write_index_, 0};

    if( H5DOwrite_chunk(
            image_dataset_.getId(),
            H5P_DEFAULT,
            0,
            image_offset,
            MODULE_N_BYTES * n_modules_ * WRITER_N_FRAMES_BUFFER,
            data))
    {
        throw runtime_error("Cannot write image dataset.");
    }

    if( H5DOwrite_chunk(
            pulse_id_dataset_.getId(),
            H5P_DEFAULT,
            0,
            metadata_offset,
            sizeof(uint64_t) * WRITER_N_FRAMES_BUFFER,
            pulse_id_data))
    {
        throw runtime_error("Cannot write pulse_id dataset.");
    }

    if( H5DOwrite_chunk(
            frame_index_dataset_.getId(),
            H5P_DEFAULT,
            0,
            metadata_offset,
            sizeof(uint64_t) * WRITER_N_FRAMES_BUFFER,
            frame_index_data))
    {
        throw runtime_error("Cannot write frame_index dataset.");
    }

    if( H5DOwrite_chunk(
            daq_rec_dataset_.getId(),
            H5P_DEFAULT,
            0,
            metadata_offset,
            sizeof(uint32_t) * WRITER_N_FRAMES_BUFFER,
            daq_rec_data))
    {
        throw runtime_error("Cannot write daq_rec dataset.");
    }

    if( H5DOwrite_chunk(
            n_received_packets_dataset_.getId(),
            H5P_DEFAULT,
            0,
            metadata_offset,
            sizeof(uint16_t) * WRITER_N_FRAMES_BUFFER,
            n_received_packets_data))
    {
        throw runtime_error("Cannot write n_received_packets dataset.");
    }

    current_write_index_ += WRITER_N_FRAMES_BUFFER;
}
