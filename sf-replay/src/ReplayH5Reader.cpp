#include "ReplayH5Reader.hpp"

#include "BufferUtils.hpp"

using namespace std;
using namespace core_buffer;

ReplayH5Reader::ReplayH5Reader(
        const string device,
        const string channel_name) :
            device_(device),
            channel_name_(channel_name)
{
}

ReplayH5Reader::~ReplayH5Reader()
{
    close_file();
}

void ReplayH5Reader::close_file()
{
    if (current_file_.getId() != -1) {
        dset_metadata_.close();
        dset_frame_.close();
        current_file_.close();
    }
}

void ReplayH5Reader::get_buffer(
        const uint64_t pulse_id,
        ReplayBuffer* metadata,
        char* data)
{
    auto pulse_filename = BufferUtils::get_filename(
            device_, channel_name_, pulse_id);

    if (pulse_filename != current_filename_) {
        close_file();

        current_filename_ = pulse_filename;
        current_file_ = H5::H5File(current_filename_, H5F_ACC_RDONLY);

        dset_metadata_ = current_file_.openDataSet(BUFFER_H5_METADATA_DATASET);
        dset_frame_ = current_file_.openDataSet(BUFFER_H5_FRAME_DATASET);
    }

    auto file_index = BufferUtils::get_file_frame_index(pulse_id);
    auto cache_start_index = file_index / BUFFER_BLOCK_SIZE;
    cache_start_index *= BUFFER_BLOCK_SIZE;

    uint64_t b_start_pulse_id = pulse_id - (file_index - cache_start_index);
    metadata->start_pulse_id = b_start_pulse_id;
    metadata->n_frames = BUFFER_BLOCK_SIZE;

    hsize_t b_m_dims[2] = {BUFFER_BLOCK_SIZE, ModuleFrame_N_FIELDS};
    H5::DataSpace b_m_space (2, b_m_dims);
    hsize_t b_m_count[] = {BUFFER_BLOCK_SIZE, ModuleFrame_N_FIELDS};
    hsize_t b_m_start[] = {cache_start_index, 0};
    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start);

    hsize_t f_m_dims[2] = {FILE_MOD, ModuleFrame_N_FIELDS};
    H5::DataSpace f_m_space (2, f_m_dims);
    hsize_t f_m_count[] = {BUFFER_BLOCK_SIZE, ModuleFrame_N_FIELDS};
    hsize_t pulse_id_start[] = {cache_start_index, 0};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, pulse_id_start);

    dset_metadata_.read(
            &metadata->metadata[0],
            H5::PredType::NATIVE_UINT64, b_m_space, f_m_space);

    hsize_t b_f_dims[3] =
            {BUFFER_BLOCK_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace b_f_space (3, b_f_dims);
    hsize_t b_f_count[] =
            {BUFFER_BLOCK_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t b_f_start[] = {0, 0, 0};
    b_f_space.selectHyperslab(H5S_SELECT_SET, b_f_count, b_f_start);

    hsize_t f_frame_dims[3] = {FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace f_f_space (3, f_frame_dims);
    hsize_t f_f_count[] =
            {BUFFER_BLOCK_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t f_f_start[] = {cache_start_index, 0, 0};
    f_f_space.selectHyperslab(H5S_SELECT_SET, f_f_count, f_f_start);

    dset_frame_.read(
            data, H5::PredType::NATIVE_UINT16, b_f_space, f_f_space);
}
