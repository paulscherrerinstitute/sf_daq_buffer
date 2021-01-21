#ifndef IMAGEBINARYWRITER_HPP
#define IMAGEBINARYWRITER_HPP

#include <string>

#include "formats.hpp"



class ImageBinaryWriter {
    const size_t IMAGE_BYTES;
    const size_t IMAGE_SLOT_BYTES;
    const size_t MAX_FILE_BYTES;
    const std::string detector_folder_;
    std::string latest_filename_;

    std::string current_output_filename_;
    int output_file_fd_;

    void open_file(const std::string& filename);
    void close_current_file();


public:
    ImageBinaryWriter(
            const std::string& detector_folder,
            const uint64_t image_n_bytes);

    virtual ~ImageBinaryWriter();

    void write(const ImageMetadata meta, const char* data);

};


#endif //IMAGEBINARYWRITER_HPP
