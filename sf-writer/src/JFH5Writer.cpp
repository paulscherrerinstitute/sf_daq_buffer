#include "JFH5Writer.hpp"

#include <sstream>
#include <cstring>
#include <hdf5_hl.h>

#include "writer_config.hpp"
#include "buffer_config.hpp"

//extern "C"
//{
//    #include "H5DOpublic.h"
//    #include <bitshuffle/bshuf_h5filter.h>
//}

using namespace std;
using namespace writer_config;
using namespace buffer_config;

JFH5Writer::JFH5Writer(const string& output_file,
                       const string& detector_name,
                       const size_t n_modules,
                       const uint64_t start_pulse_id,
                       const uint64_t stop_pulse_id,
                       const size_t pulse_id_step) :
        detector_name_(detector_name),
        n_modules_(n_modules),
        start_pulse_id_(start_pulse_id),
        stop_pulse_id_(stop_pulse_id),
        pulse_id_step_(pulse_id_step),
        n_images_(get_n_pulses_in_range(start_pulse_id,
                                        stop_pulse_id,
                                        pulse_id_step)),
        n_total_pulses_(stop_pulse_id_ - start_pulse_id_ + 1),
        meta_write_index_(0),
        data_write_index_(0)
{

//    bshuf_register_h5filter();

    file_ = H5::H5File(output_file, H5F_ACC_TRUNC);
    file_.createGroup("/data");
    file_.createGroup("/data/" + detector_name_);

    hsize_t image_dataset_dims[3] =
            {n_images_, n_modules * MODULE_Y_SIZE, MODULE_X_SIZE};

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
            "/data/" + detector_name_ + "/data",
            H5::PredType::NATIVE_UINT16,
            image_dataspace,
            image_dataset_properties);

    b_pulse_id_ = new uint64_t[n_total_pulses_];
    b_frame_index_= new uint64_t[n_total_pulses_];
    b_daq_rec_ = new uint32_t[n_total_pulses_];
    b_is_good_frame_ = new uint8_t[n_total_pulses_];
}

JFH5Writer::~JFH5Writer()
{
    close_file();

    delete[] b_pulse_id_;
    delete[] b_frame_index_;
    delete[] b_daq_rec_;
    delete[] b_is_good_frame_;
}

size_t JFH5Writer::get_n_pulses_in_range(
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id,
        const int pulse_id_step)
{
    if (stop_pulse_id < start_pulse_id) {
        throw runtime_error("stop_pulse_id smaller than start_pulse_id.");
    }

    if (100 % pulse_id_step != 0) {
        throw runtime_error("100 is not divisible by the pulse_id_step.");
    }

    if (start_pulse_id % pulse_id_step != 0) {
        throw runtime_error("start_pulse_id not divisible by pulse_id_step.");
    }

    if (stop_pulse_id % pulse_id_step != 0) {
        throw runtime_error("stop_pulse_id not divisible by pulse_id_step.");
    }

    size_t n_pulses = 1;
    n_pulses += (stop_pulse_id / pulse_id_step);
    n_pulses -= start_pulse_id / pulse_id_step;

    if (n_pulses == 0) {
        throw runtime_error("Zero pulses to write in given range.");
    }

    return n_pulses;
}

void JFH5Writer::write_metadata()
{
    hsize_t b_m_dims[] = {n_total_pulses_};
    hsize_t b_m_count[] = {n_images_};
    hsize_t b_m_start[] = {0};
    hsize_t b_m_stride[] = {pulse_id_step_};
    H5::DataSpace b_m_space (1, b_m_dims);
    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start, b_m_stride);

    hsize_t f_m_dims[] = {n_images_, 1};
    H5::DataSpace f_m_space(2, f_m_dims);

    auto pulse_id_dataset = file_.createDataSet(
            "/data/" + detector_name_ + "/pulse_id",
            H5::PredType::NATIVE_UINT64, f_m_space);
    pulse_id_dataset.write(
            b_pulse_id_, H5::PredType::NATIVE_UINT64, b_m_space, f_m_space);
    pulse_id_dataset.close();

    auto frame_index_dataset = file_.createDataSet(
            "/data/" + detector_name_ + "/frame_index",
            H5::PredType::NATIVE_UINT64, f_m_space);
    frame_index_dataset.write(
            b_frame_index_, H5::PredType::NATIVE_UINT64, b_m_space, f_m_space);
    frame_index_dataset.close();

    auto daq_rec_dataset = file_.createDataSet(
            "/data/" + detector_name_ + "/daq_rec",
            H5::PredType::NATIVE_UINT32, f_m_space);
    daq_rec_dataset.write(
            b_daq_rec_, H5::PredType::NATIVE_UINT32, b_m_space, f_m_space);
    daq_rec_dataset.close();

    auto is_good_frame_dataset = file_.createDataSet(
            "/data/" + detector_name_ + "/is_good_frame",
            H5::PredType::NATIVE_UINT8, f_m_space);
    is_good_frame_dataset.write(
            b_is_good_frame_, H5::PredType::NATIVE_UINT8, b_m_space, f_m_space);
    is_good_frame_dataset.close();
}

void JFH5Writer::close_file()
{
    if (file_.getId() == -1) {
        return;
    }

    image_dataset_.close();

    write_metadata();

    file_.close();
}

void JFH5Writer::write(
        const ImageMetadataBlock* metadata, const char* data)
{
    size_t n_images_offset = 0;
    if (start_pulse_id_ > metadata->block_start_pulse_id) {
        n_images_offset = start_pulse_id_ - metadata->block_start_pulse_id;
    }

    if (n_images_offset > BUFFER_BLOCK_SIZE) {
        throw runtime_error("Received unexpected block for start_pulse_id.");
    }

    size_t n_images_to_copy = BUFFER_BLOCK_SIZE - n_images_offset;
    if (stop_pulse_id_ < metadata->block_stop_pulse_id) {
        n_images_to_copy -= metadata->block_stop_pulse_id - stop_pulse_id_;
    }

    if (n_images_to_copy < 1) {
        throw runtime_error("Received unexpected block for stop_pulse_id.");
    }

//    hsize_t b_i_dims[3] = {BUFFER_BLOCK_SIZE,
//                           MODULE_Y_SIZE * n_modules_,
//                           MODULE_X_SIZE};
//    H5::DataSpace b_i_space(3, b_i_dims);
//    hsize_t b_i_count[] = {n_images_to_copy,
//                           MODULE_Y_SIZE * n_modules_,
//                           MODULE_X_SIZE};
//    hsize_t b_i_start[] = {n_images_offset, 0, 0};
//    b_i_space.selectHyperslab(H5S_SELECT_SET, b_i_count, b_i_start);
//
//    hsize_t f_i_dims[3] = {n_images_,
//                           MODULE_Y_SIZE * n_modules_,
//                           MODULE_X_SIZE};
//    H5::DataSpace f_i_space(3, f_i_dims);
//    hsize_t f_i_count[] = {n_images_to_copy,
//                           MODULE_Y_SIZE * n_modules_,
//                           MODULE_X_SIZE};
//    hsize_t f_i_start[] = {data_write_index_, 0, 0};
//    f_i_space.selectHyperslab(H5S_SELECT_SET, f_i_count, f_i_start);
//
//    image_dataset_.write(
//            data, H5::PredType::NATIVE_UINT16, b_i_space, f_i_space);

    // TODO: Can the i_image++ be made more efficient?
    for (size_t i_image=n_images_offset;
         i_image < n_images_offset + n_images_to_copy;
         i_image++) {

        if (i_image % pulse_id_step_ != 0) {
            continue;
        }

        hsize_t offset[] = {data_write_index_, 0, 0};
        H5DOwrite_chunk(
                image_dataset_.getId(),
                H5P_DEFAULT,
                0,
                offset,
                MODULE_N_BYTES * n_modules_,
                data);

        data_write_index_++;
    }

    // pulse_id
    {
        auto b_current_ptr = b_pulse_id_ + meta_write_index_;
        memcpy(b_current_ptr,
               &(metadata->pulse_id[n_images_offset]),
               sizeof(uint64_t) * n_images_to_copy);
    }

    // frame_index
    {
        auto b_current_ptr = b_frame_index_ + meta_write_index_;
        memcpy(b_current_ptr,
               &(metadata->frame_index[n_images_offset]),
               sizeof(uint64_t) * n_images_to_copy);
    }

    // daq_rec
    {
        auto b_current_ptr = b_daq_rec_ + meta_write_index_;
        memcpy(b_current_ptr,
               &(metadata->daq_rec[n_images_offset]),
               sizeof(uint32_t) * n_images_to_copy);
    }

    // is_good_frame
    {
        auto b_current_ptr = b_is_good_frame_ + meta_write_index_;
        memcpy(b_current_ptr,
               &(metadata->is_good_image[n_images_offset]),
               sizeof(uint8_t) * n_images_to_copy);
    }

    meta_write_index_ += n_images_to_copy;
}
