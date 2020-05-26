#include "JFH5Writer.hpp"
#include <sstream>
#include <cstring>


//extern "C"
//{
//    #include "H5DOpublic.h"
//    #include <bitshuffle/bshuf_h5filter.h>
//}

using namespace std;
using namespace core_buffer;

JFH5Writer::JFH5Writer(
        const string& output_file,
        const size_t n_frames,
        const size_t n_modules) :
        n_frames_(n_frames),
        n_modules_(n_modules),
        current_write_index_(0)
{

//    bshuf_register_h5filter();

    file_ = H5::H5File(output_file, H5F_ACC_TRUNC);

    hsize_t image_dataset_dims[3] =
            {n_frames_, n_modules * MODULE_Y_SIZE, MODULE_X_SIZE};

    H5::DataSpace image_dataspace(3, image_dataset_dims);

    hsize_t image_dataset_chunking[3] =
            {1, n_modules * MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DSetCreatPropList image_dataset_properties;
    image_dataset_properties.setChunk(3, image_dataset_chunking);

//    // block_size, compression type
//    uint compression_prop[] =
//            {MODULE_N_PIXELS, //block size
//             BSHUF_H5_COMPRESS_LZ4}; // Compression type
//
//    H5Pset_filter(image_dataset_properties.getId(),
//            BSHUF_H5FILTER,
//            H5Z_FLAG_MANDATORY,
//            2,
//            &(compression_prop[0]));

    image_dataset_ = file_.createDataSet(
            "image",
            H5::PredType::NATIVE_UINT16,
            image_dataspace,
            image_dataset_properties);

    hsize_t metadata_dataset_dims[] = {n_frames_, 1};
    H5::DataSpace metadata_dataspace(2, metadata_dataset_dims);

    pulse_id_dataset_ = file_.createDataSet(
            "pulse_id",
            H5::PredType::NATIVE_UINT64,
            metadata_dataspace);

    frame_index_dataset_ = file_.createDataSet(
            "frame_index",
            H5::PredType::NATIVE_UINT64,
            metadata_dataspace);

    daq_rec_dataset_ = file_.createDataSet(
            "daq_rec",
            H5::PredType::NATIVE_UINT32,
            metadata_dataspace);

    is_good_frame_dataset_ = file_.createDataSet(
            "is_good_frame",
            H5::PredType::NATIVE_UINT8,
            metadata_dataspace);

    b_pulse_id_ = new uint64_t[n_frames_];
    b_frame_index_= new uint64_t[n_frames_];
    b_daq_rec_ = new uint32_t[n_frames_];
    b_is_good_frame_ = new uint8_t[n_frames_];
}

JFH5Writer::~JFH5Writer()
{
    close_file();

    delete[] b_pulse_id_;
    delete[] b_frame_index_;
    delete[] b_daq_rec_;
    delete[] b_is_good_frame_;
}

void JFH5Writer::close_file()
{

    pulse_id_dataset_.write(b_pulse_id_, H5::PredType::NATIVE_UINT64);

    frame_index_dataset_.write(b_frame_index_,
                               H5::PredType::NATIVE_UINT64);

    daq_rec_dataset_.write(b_daq_rec_, H5::PredType::NATIVE_UINT32);

    is_good_frame_dataset_.write(b_is_good_frame_,
                                 H5::PredType::NATIVE_UINT8);

    image_dataset_.close();
    pulse_id_dataset_.close();
    frame_index_dataset_.close();
    daq_rec_dataset_.close();
    is_good_frame_dataset_.close();

    file_.close();
}

void JFH5Writer::write(
        const ImageMetadataBuffer* metadata, const char* data)
{
    auto n_images_in_buffer = metadata->n_images;

    hsize_t b_i_dims[3] = {
            n_images_in_buffer,
            MODULE_Y_SIZE*n_modules_,
            MODULE_X_SIZE};
    H5::DataSpace b_i_space(3, b_i_dims);

    hsize_t f_i_dims[3] = {n_frames_,
                           MODULE_Y_SIZE * n_modules_,
                           MODULE_X_SIZE};
    H5::DataSpace f_i_space(3, f_i_dims);

    hsize_t i_count[] = {n_images_in_buffer,
                         MODULE_Y_SIZE*n_modules_,
                         MODULE_X_SIZE};
    hsize_t i_start[] = {current_write_index_, 0, 0};
    f_i_space.selectHyperslab(H5S_SELECT_SET, i_count, i_start);

    image_dataset_.write(
            data, H5::PredType::NATIVE_UINT16,
            b_i_space, f_i_space);

    {
        auto b_current_ptr = b_pulse_id_ + current_write_index_;
        const uint64_t* metadata_pulse_id_ptr =
                &(metadata->pulse_id[0]) + current_write_index_;
        memcpy(b_current_ptr,
               metadata_pulse_id_ptr,
               sizeof(uint64_t) * n_images_in_buffer);
    }

    {
        auto b_current_ptr = b_frame_index_ + current_write_index_;
        const uint64_t* metadata_pulse_id_ptr =
                &(metadata->frame_index[0]) + current_write_index_;
        memcpy(b_current_ptr,
               metadata_pulse_id_ptr,
               sizeof(uint64_t) * n_images_in_buffer);
    }

    {
        auto b_current_ptr = b_daq_rec_ + current_write_index_;
        const uint32_t* metadata_pulse_id_ptr =
                &(metadata->daq_rec[0]) + current_write_index_;
        memcpy(b_current_ptr,
               metadata_pulse_id_ptr,
               sizeof(uint32_t) * n_images_in_buffer);
    }

    {
        auto b_current_ptr = b_is_good_frame_ + current_write_index_;
        const uint8_t* metadata_pulse_id_ptr =
                &(metadata->is_good_image[0]) + current_write_index_;
        memcpy(b_current_ptr,
               metadata_pulse_id_ptr,
               sizeof(uint8_t) * n_images_in_buffer);
    }

    current_write_index_ += n_images_in_buffer;
}
