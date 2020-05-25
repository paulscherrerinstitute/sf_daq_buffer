#include "ReplayH5Reader.hpp"

#include "BufferUtils.hpp"

using namespace std;
using namespace core_buffer;

void ReplayH5Reader::load_buffers(const uint64_t pulse_id)
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
    auto cache_start_index = file_index / REPLAY_READ_BUFFER_SIZE;
    cache_start_index *= REPLAY_READ_BUFFER_SIZE;

    buffer_start_pulse_id_ = pulse_id - (file_index - cache_start_index);
    buffer_end_pulse_id_ = buffer_start_pulse_id_ + REPLAY_READ_BUFFER_SIZE - 1;

    hsize_t b_m_dims[2] = {REPLAY_READ_BUFFER_SIZE, ModuleFrame_N_FIELDS};
    H5::DataSpace b_m_space (2, b_m_dims);
    hsize_t b_m_count[] = {REPLAY_READ_BUFFER_SIZE, ModuleFrame_N_FIELDS};
    hsize_t b_m_start[] = {0, 0};
    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start);

    hsize_t f_m_dims[2] = {FILE_MOD, ModuleFrame_N_FIELDS};
    H5::DataSpace f_m_space (2, f_m_dims);
    hsize_t f_m_count[] = {REPLAY_READ_BUFFER_SIZE, ModuleFrame_N_FIELDS};
    hsize_t pulse_id_start[] = {cache_start_index, 0};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, pulse_id_start);

    dset_metadata_.read(
            m_buffer_, H5::PredType::NATIVE_UINT64, b_m_space, f_m_space);

    hsize_t b_f_dims[3] =
            {REPLAY_READ_BUFFER_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace b_f_space (3, b_f_dims);
    hsize_t b_f_count[] =
            {REPLAY_READ_BUFFER_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t b_f_start[] = {0, 0, 0};
    b_f_space.selectHyperslab(H5S_SELECT_SET, b_f_count, b_f_start);

    hsize_t f_frame_dims[3] = {FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace f_f_space (3, f_frame_dims);
    hsize_t f_f_count[] =
            {REPLAY_READ_BUFFER_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t f_f_start[] = {cache_start_index, 0, 0};
    f_f_space.selectHyperslab(H5S_SELECT_SET, f_f_count, f_f_start);

    dset_frame_.read(
            f_buffer_, H5::PredType::NATIVE_UINT16, b_f_space, f_f_space);
}

ReplayH5Reader::ReplayH5Reader(
        const string device,
        const string channel_name) :
            device_(device),
            channel_name_(channel_name)
{
    m_buffer_ = new ModuleFrame[REPLAY_READ_BUFFER_SIZE];
    f_buffer_ = new char[MODULE_N_BYTES * REPLAY_READ_BUFFER_SIZE];
}

ReplayH5Reader::~ReplayH5Reader()
{
    close_file();

    delete[] m_buffer_;
    delete[] f_buffer_;
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
        ModuleFrame*& metadata,
        char*& data)
{
    // Buffer start and end pulse_ids are inclusive.
    if ((pulse_id < buffer_start_pulse_id_) ||
        (pulse_id > buffer_end_pulse_id_)) {
        load_buffers(pulse_id);
    }

    auto buffer_index = pulse_id - buffer_start_pulse_id_;

    metadata = m_buffer_ + (buffer_index * sizeof(ModuleFrame));
    data = f_buffer_ + (buffer_index * MODULE_N_BYTES);
}
