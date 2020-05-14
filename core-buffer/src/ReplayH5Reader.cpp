#include "ReplayH5Reader.hpp"

#include "BufferUtils.hpp"
#include "buffer_config.hpp"
#include <iostream>
#include <chrono>
#include "date.h"

using namespace std;
using namespace core_buffer;

void ReplayH5Reader::prepare_file_for_pulse(const uint64_t pulse_id)
{
    auto pulse_filename = BufferUtils::get_filename(
            device_, channel_name_, pulse_id);

    if (pulse_filename == current_filename_) {
        return;
    }

    close_file();

    current_filename_ = pulse_filename;
    current_file_ = H5::H5File(current_filename_, H5F_ACC_RDONLY);

    dset_metadata_ = current_file_.openDataSet(BUFFER_H5_METADATA_DATASET);
    dset_frame_ = current_file_.openDataSet(BUFFER_H5_FRAME_DATASET);
}

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

bool ReplayH5Reader::get_frame(
        const uint64_t pulse_id, ModuleFrame* metadata, char* frame_buffer)
{
    prepare_file_for_pulse(pulse_id);

    auto index_in_file = BufferUtils::get_file_frame_index(pulse_id);

    hsize_t b_metadata_dims[2] = {1, ModuleFrame_N_FIELDS};
    H5::DataSpace b_m_space (2, b_metadata_dims);
    hsize_t b_m_count[] = {1, ModuleFrame_N_FIELDS};
    hsize_t b_m_start[] = {0, 0};
    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start);

    hsize_t f_metadata_dims[2] = {FILE_MOD, ModuleFrame_N_FIELDS};
    H5::DataSpace f_m_space (2, f_metadata_dims);
    hsize_t f_m_count[] = {1, ModuleFrame_N_FIELDS};
    hsize_t f_m_start[] = {index_in_file, 0};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, f_m_start);

    dset_metadata_.read(metadata, H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    hsize_t b_image_dims[3] = {1, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace b_f_space (3, b_image_dims);
    hsize_t b_i_count[] = {1, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t b_i_start[] = {0, 0, 0};
    b_f_space.selectHyperslab(H5S_SELECT_SET, b_i_count, b_i_start);

    hsize_t f_frame_dims[3] = {FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace f_f_space (3, f_frame_dims);
    hsize_t f_f_count[] = {1, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t f_f_start[] = {index_in_file, 0, 0};
    f_f_space.selectHyperslab(H5S_SELECT_SET, f_f_count, f_f_start);

    dset_frame_.read(frame_buffer, H5::PredType::NATIVE_UINT16,
            b_f_space, f_f_space);

    if (metadata->pulse_id == 0) {
        // Signal that there is no frame at this pulse_id.
        return false;

    }else if (metadata->pulse_id != pulse_id) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[ReplayH5Reader::get_frame]";
        err_msg << " Corrupted file " << current_filename_;
        err_msg << " index_in_file " << index_in_file;
        err_msg << " expected pulse_id " << pulse_id;
        err_msg << " but read " << metadata->pulse_id << endl;

        throw runtime_error(err_msg.str());
    }

    return true;
}
