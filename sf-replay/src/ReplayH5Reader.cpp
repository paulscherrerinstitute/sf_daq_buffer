#include "ReplayH5Reader.hpp"

#include "BufferUtils.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include "date.h"

using namespace std;
using namespace core_buffer;

void ReplayH5Reader::load_buffers(
        const uint64_t pulse_id,
        const size_t n_pulses,
        ReplayModuleFrameBuffer* metadata,
        char* frame_buffer)
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
        const uint16_t source_id,
        const uint64_t stop_pulse_id) :
            device_(device),
            channel_name_(channel_name),
            source_id_(source_id),
            stop_pulse_id_(stop_pulse_id)
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

bool ReplayH5Reader::get_buffer(
        const uint64_t pulse_id,
        ReplayModuleFrameBuffer* metadata,
        char* frame_buffer)
{
    auto start_pulse_id = pulse_id;
    auto n_pulses = REPLAY_READ_BUFFER_SIZE;
    auto buffer_end_pulse_id = start_pulse_id + n_pulses - 1;

    // The last read segment might not fill the complete buffer.
    if (stop_pulse_id_ < buffer_end_pulse_id) {
        // stop_pulse_id_ must be included in the stream.
        buffer_end_pulse_id = stop_pulse_id_;
        n_pulses = buffer_end_pulse_id - start_pulse_id + 1;
    }

    load_buffers(pulse_id, n_pulses, metadata, frame_buffer);

    for (size_t i_frame=0; i_frame<n_pulses; i_frame++) {
        auto curr_pulse_id = start_pulse_id + i_frame;

        metadata->is_frame_present[i_frame] = true;
        if (metadata->pulse_id[i_frame] == 0) {
            // Writer expects all pulse_ids, even ones with no data.
            metadata->pulse_id[i_frame] = curr_pulse_id;
            metadata->is_frame_present[i_frame] = false;
        }

        // TODO: This looks really ugly. Fix condition length.
        metadata->is_good_frame[i_frame] = true;
        if (!metadata->is_frame_present[i_frame] ||
            metadata->n_received_packets[i_frame] !=
                JUNGFRAU_N_PACKETS_PER_FRAME) {
            metadata->is_good_frame[i_frame] = false;
        }

        if (metadata->pulse_id[i_frame] != curr_pulse_id) {
            stringstream err_msg;

            using namespace date;
            using namespace chrono;
            err_msg << "[" << system_clock::now() << "]";
            err_msg << "[ReplayH5Reader::get_buffer]";
            err_msg << " Corrupted file " << current_filename_;
            err_msg << " expected pulse_id " << curr_pulse_id;
            err_msg << " but read " << metadata->pulse_id[i_frame] << endl;

            throw runtime_error(err_msg.str());
        }
    }

    metadata->module_id = source_id_;
    metadata->data_n_bytes = n_pulses * MODULE_N_BYTES;
    metadata->n_frames = n_pulses;

    return true;
}
