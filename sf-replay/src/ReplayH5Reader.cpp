#include "ReplayH5Reader.hpp"

#include "BufferUtils.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include "date.h"

using namespace std;
using namespace core_buffer;

void ReplayH5Reader::load_buffers(const uint64_t pulse_id)
{
    auto pulse_filename = BufferUtils::get_filename(
            device_, channel_name_, pulse_id);
    auto start_index_in_file = BufferUtils::get_file_frame_index(pulse_id);

    if (pulse_filename != current_filename_) {
        close_file();

        current_filename_ = pulse_filename;
        current_file_ = H5::H5File(current_filename_, H5F_ACC_RDONLY);

        dset_metadata_ = current_file_.openDataSet(BUFFER_H5_METADATA_DATASET);
        dset_frame_ = current_file_.openDataSet(BUFFER_H5_FRAME_DATASET);
    }

    hsize_t b_m_dims[2] = {n_pulses, 1};
    H5::DataSpace b_m_space (2, b_m_dims);
    hsize_t b_m_count[] = {n_pulses, 1};
    hsize_t b_m_start[] = {0, 0};
    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start);

    hsize_t f_m_dims[2] = {FILE_MOD, ModuleFrame_N_FIELDS};
    H5::DataSpace f_m_space (2, f_m_dims);
    hsize_t f_m_count[] = {n_pulses, 1};

    hsize_t pulse_id_start[] = {start_index_in_file, 0};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, pulse_id_start);
    dset_metadata_.read(
            &(metadata->pulse_id[0]), H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    hsize_t frame_index_start[] = {start_index_in_file, 1};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, frame_index_start);
    dset_metadata_.read(
            &(metadata->frame_index[0]), H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    hsize_t daq_rec_start[] = {start_index_in_file, 2};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, daq_rec_start);
    dset_metadata_.read(
            &(metadata->daq_rec[0]), H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    hsize_t n_packets_start[] = {start_index_in_file, 3};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, n_packets_start);
    dset_metadata_.read(
            &(metadata->n_received_packets[0]), H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    hsize_t b_f_dims[3] =
            {n_pulses, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace b_f_space (3, b_f_dims);
    hsize_t b_f_count[] =
            {n_pulses, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t b_f_start[] = {0, 0, 0};
    b_f_space.selectHyperslab(H5S_SELECT_SET, b_f_count, b_f_start);

    hsize_t f_frame_dims[3] = {FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace f_f_space (3, f_frame_dims);
    hsize_t f_f_count[] =
            {n_pulses, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t f_f_start[] = {start_index_in_file, 0, 0};
    f_f_space.selectHyperslab(H5S_SELECT_SET, f_f_count, f_f_start);

    dset_frame_.read(
            frame_buffer, H5::PredType::NATIVE_UINT16, b_f_space, f_f_space);
}

ReplayH5Reader::ReplayH5Reader(
        const string device,
        const string channel_name,
        const uint16_t source_id) :
            device_(device),
            channel_name_(channel_name),
            source_id_(source_id)
{
    m_buffer = new ModuleFrame[REPLAY_READ_BUFFER_SIZE];
    f_buffer = new char[MODULE_N_BYTES * REPLAY_READ_BUFFER_SIZE];
}

ReplayH5Reader::~ReplayH5Reader()
{
    close_file();

    delete[] m_buffer;
    delete[] f_buffer;
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

    metadata = m_buffer + (buffer_index * sizeof(ModuleFrame));
    data = f_buffer + (buffer_index * MODULE_N_BYTES);
}
