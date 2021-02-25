#include "JFH5Writer.hpp"

#include <sstream>
#include <cstring>
#include <H5version.h>


#include "live_writer_config.hpp"
#include "buffer_config.hpp"
#include "formats.hpp"

extern "C"
{
    #include "H5DOpublic.h"
    #include <bitshuffle/bshuf_h5filter.h>
}

using namespace std;
using namespace buffer_config;
using namespace live_writer_config;

JFH5Writer::JFH5Writer(const BufferUtils::DetectorConfig config):
        root_folder_(config.buffer_folder),
        detector_name_(config.detector_name),
{
}

JFH5Writer::~JFH5Writer()
{
    close_file();
}



void JFH5Writer::open_run(const int64_t run_id, const uint32_t n_images)
{
    close_file();

    const string output_folder = root_folder_ + "/" + OUTPUT_FOLDER_SYMLINK;
    // TODO: Maybe add leading zeros to filename?
    const string output_file = output_folder + to_string(run_id) + ".h5";

    open_file(output_file, n_images);

    current_run_id_ = run_id;
}

void JFH5Writer::close_run()
{
    close_file();
    current_run_id_ = NO_RUN_ID;
}

void JFH5Writer::open_file(const string& output_file, const uint32_t n_images)
{

    // Create file
    auto fcpl_id = H5Pcreate(H5P_FILE_ACCESS);
    if (fcpl_id == -1) {
        throw runtime_error("Error in file access property list.");
    }

    if (H5Pset_fapl_mpio(fcpl_id, MPI_COMM_WORLD, MPI_INFO_NULL) < 0) {
        throw runtime_error("Cannot set mpio to property list.");
    }

    file_id_ = H5Fcreate(
            output_file.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, fcpl_id);
    if (file_id_ < 0) {
        throw runtime_error("Cannot create output file.");
    }

    H5Pclose(fcpl_id);

    // Create group
    auto data_group_id = H5Gcreate(file_id_, detector_name_.c_str(),
                                   H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (data_group_id < 0) {
        throw runtime_error("Cannot create data group.");
    }

    // Create image dataset.
    auto dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
    if (dcpl_id < 0) {
        throw runtime_error("Error in creating dataset create property list.");
    }

    hsize_t image_dataset_chunking[] = {1, image_y_size_, image_x_size_};
    if (H5Pset_chunk(dcpl_id, 3, image_dataset_chunking) < 0) {
        throw runtime_error("Cannot set image dataset chunking.");
    }

    if (H5Pset_fill_time(dcpl_id, H5D_FILL_TIME_NEVER) < 0) {
        throw runtime_error("Cannot set image dataset fill time.");
    }

    if (H5Pset_alloc_time(dcpl_id, H5D_ALLOC_TIME_EARLY) < 0) {
        throw runtime_error("Cannot set image dataset allocation time.");
    }

    hsize_t image_dataset_dims[] = {n_images, image_y_size_, image_x_size_};
    auto image_space_id = H5Screate_simple(3, image_dataset_dims, NULL);
    if (image_space_id < 0) {
        throw runtime_error("Cannot create image dataset space.");
    }

    // TODO: Enable compression.
//    bshuf_register_h5filter();
//    uint filter_prop[] = {PIXEL_N_BYTES, BSHUF_H5_COMPRESS_LZ4};
//    if (H5Pset_filter(dcpl_id, BSHUF_H5FILTER, H5Z_FLAG_MANDATORY,
//                      2, filter_prop) < 0) {
//        throw runtime_error("Cannot set compression filter on dataset.");
//    }

    image_dataset_id_ = H5Dcreate(
            data_group_id, "data", H5T_NATIVE_INT, image_space_id,
            H5P_DEFAULT, dcpl_id, H5P_DEFAULT);
    if (image_dataset_id_ < 0) {
        throw runtime_error("Cannot create image dataset.");
    }

    // Create metadata datasets.
    hsize_t meta_dataset_dims[] = {n_images};
    auto meta_space_id = H5Screate_simple(1, meta_dataset_dims, NULL);
    if (meta_space_id < 0) {
        throw runtime_error("Cannot create meta dataset space.");
    }

    auto create_meta_dataset = [&](string name, hid_t data_type) {
        auto dataset_id = H5Dcreate(
                data_group_id, name.c_str(), data_type, meta_space_id,
                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (dataset_id < 0) {
            throw runtime_error("Cannot create " + name + " dataset.");
        }

        return dataset_id;
    };

    pulse_dataset_id_ = create_meta_dataset("pulse_id", H5T_NATIVE_UINT64);
    frame_dataset_id_ = create_meta_dataset("frame_index", H5T_NATIVE_UINT64);
    daq_rec_dataset_id_ = create_meta_dataset("daq_rec", H5T_NATIVE_UINT32);
    is_good_dataset_id_ = create_meta_dataset("is_good_frame", H5T_NATIVE_UINT8);

    H5Sclose(meta_space_id);
    H5Sclose(image_space_id);
    H5Pclose(dcpl_id);
    H5Gclose(data_group_id);
}

void JFH5Writer::close_file()
{
    if (file_id_ < 0) {
        return;
    }

    H5Dclose(image_dataset_id_);
    image_dataset_id_ = -1;

    H5Dclose(pulse_dataset_id_);
    pulse_dataset_id_ = -1;

    H5Dclose(frame_dataset_id_);
    frame_dataset_id_ = -1;

    H5Dclose(daq_rec_dataset_id_);
    daq_rec_dataset_id_ = -1;

    H5Dclose(is_good_dataset_id_);
    is_good_dataset_id_ = -1;

    H5Fclose(file_id_);
    file_id_ = -1;
}

void JFH5Writer::write_data(
        const int64_t run_id, const uint32_t index, const char* data)
{
    if (run_id != current_run_id_) {
        throw runtime_error("Invalid run_id.");
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

    hsize_t offset[] = {data_write_index_, 0, 0};
    size_t data_offset = i_image * MODULE_N_BYTES * n_modules_;

    H5DOwrite_chunk(
            image_dataset_.getId(),
            H5P_DEFAULT,
            0,
            offset,
            MODULE_N_BYTES * n_modules_,
            data + data_offset);
}

void JFH5Writer::write_meta(
        const int64_t run_id, const uint32_t index, const ImageMetadata& meta)
{
    if (run_id != current_run_id_) {
        throw runtime_error("Invalid run_id.");
    }


}
