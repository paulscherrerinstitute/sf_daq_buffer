#ifndef SFWRITER_HPP
#define SFWRITER_HPP

#include <memory>
#include <string>
#include <H5Cpp.h>

#include "LiveImageAssembler.hpp"

const auto& H5_UINT64 = H5::PredType::NATIVE_UINT64;
const auto& H5_UINT32 = H5::PredType::NATIVE_UINT32;
const auto& H5_UINT16 = H5::PredType::NATIVE_UINT16;
const auto& H5_UINT8 = H5::PredType::NATIVE_UINT8;

class JFH5LiveWriter {

    const std::string detector_name_;
    const size_t n_modules_;
    const size_t n_pulses_;

    size_t write_index_;

    H5::H5File file_;
    H5::DataSet image_dataset_;

    uint64_t* b_pulse_id_;
    uint64_t* b_frame_index_;
    uint32_t* b_daq_rec_;
    uint8_t* b_is_good_frame_ ;

    void init_file(const std::string &output_file);
    void write_dataset(const std::string name,
                       const void *buffer,
                       const H5::PredType &type);
    void write_metadata();
    std::string get_detector_name(const std::string& detector_folder);

    void close_file();

public:
    JFH5LiveWriter(const std::string& output_file,
                   const std::string& detector_folder,
                   const size_t n_modules,
                   const size_t n_pulses);
    ~JFH5LiveWriter();
    void write(const ImageMetadata* metadata, const char* data);
};

#endif //SFWRITER_HPP
