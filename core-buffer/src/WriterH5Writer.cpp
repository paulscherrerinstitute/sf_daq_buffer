#include "WriterH5Writer.hpp"
#include <sstream>


extern "C"
{
    #include "H5DOpublic.h"
    #include <bitshuffle/bshuf_h5filter.h>
}

using namespace std;
using namespace core_buffer;

WriterH5Writer::WriterH5Writer(
        const string& output_file,
        const size_t n_frames,
        const size_t n_modules,
        const size_t image_cache_n_images) :
        n_frames_(n_frames),
        n_modules_(n_modules),
        image_cache_n_images_(image_cache_n_images),
        current_write_index_(0)
{

    bshuf_register_h5filter();

    file_ = H5::H5File(output_file, H5F_ACC_TRUNC);

    hsize_t image_dataset_dims[3] =
            {n_frames_, n_modules * MODULE_Y_SIZE, MODULE_X_SIZE};

    H5::DataSpace image_dataspace(3, image_dataset_dims);

    hsize_t image_dataset_chunking[3] =
            {1, n_modules * MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DSetCreatPropList image_dataset_properties;
    image_dataset_properties.setChunk(3, image_dataset_chunking);

    // block_size, compression type
    uint compression_prop[] =
            {MODULE_N_PIXELS, //block size
             BSHUF_H5_COMPRESS_LZ4}; // Compression type

    H5Pset_filter(image_dataset_properties.getId(),
            BSHUF_H5FILTER,
            H5Z_FLAG_MANDATORY,
            2,
            &(compression_prop[0]));

    image_dataset_ = file_.createDataSet(
            "image",
            H5::PredType::NATIVE_UINT16,
            image_dataspace,
            image_dataset_properties);

    hsize_t metadata_dataset_dims[] = {n_frames_, 1};
    H5::DataSpace metadata_dataspace(2, metadata_dataset_dims);

    // Chunk cannot be larger than n_frames.
    auto metadata_chunk_size = WRITER_METADATA_CHUNK_N_IMAGES;
    if (n_frames < metadata_chunk_size) {
        metadata_chunk_size = n_frames;
    }

    hsize_t metadata_dataset_chunking[] = {metadata_chunk_size, 1};
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

    is_good_frame_dataset_ = file_.createDataSet(
            "is_good_frame",
            H5::PredType::NATIVE_UINT8,
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
    is_good_frame_dataset_.close();

    file_.close();
}

void WriterH5Writer::write(const ImageMetadata* metadata, const char* data) {

    hsize_t b_i_dims[3] = {
            image_cache_n_images_,
            MODULE_Y_SIZE*n_modules_,
            MODULE_X_SIZE};
    H5::DataSpace b_i_space(3, b_i_dims);

    hsize_t f_i_dims[3] = {n_frames_,
                           MODULE_Y_SIZE * n_modules_,
                           MODULE_X_SIZE};
    H5::DataSpace f_i_space(3, f_i_dims);
    hsize_t i_count[] = {image_cache_n_images_,
                         MODULE_Y_SIZE*n_modules_,
                         MODULE_X_SIZE};

    hsize_t i_start[] = {current_write_index_, 0, 0};
    f_i_space.selectHyperslab(H5S_SELECT_SET, i_count, i_start);

    image_dataset_.write(
            data, H5::PredType::NATIVE_UINT16,
            b_i_space, f_i_space);

    hsize_t b_m_dims[1] = {1};
    H5::DataSpace b_m_space (1, b_m_dims);

    hsize_t f_m_dims[] = {n_frames_, 1};
    H5::DataSpace f_m_space(2, f_m_dims);
    hsize_t meta_count[] = {1, 1};
    hsize_t meta_start[] = {current_write_index_, 0};
    f_m_space.selectHyperslab(H5S_SELECT_SET, meta_count, meta_start);

    pulse_id_dataset_.write(
            &(metadata->pulse_id), H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    frame_index_dataset_.write(
            &(metadata->frame_index), H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    daq_rec_dataset_.write(
            &(metadata->daq_rec), H5::PredType::NATIVE_UINT32,
            b_m_space, f_m_space);

    is_good_frame_dataset_.write(
            &(metadata->is_good_frame), H5::PredType::NATIVE_UINT8,
            b_m_space, f_m_space);

    current_write_index_++;
}
