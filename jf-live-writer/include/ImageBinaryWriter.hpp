#ifndef BINARYWRITER_HPP
#define BINARYWRITER_HPP

#include <string>

#include "formats.hpp"

class ImageBinaryWriter {

    const size_t MAX_FILE_BYTES =
            buffer_config::FILE_MOD * sizeof(BufferBinaryFormat);

    const std::string detector_folder_;
    std::string latest_filename_;

    std::string current_output_filename_;
    int output_file_fd_;

    void open_file(const std::string& filename);
    void close_current_file();


public:
    ImageBinaryWriter(const std::string& detector_folder);

    virtual ~ImageBinaryWriter();

    void write(const uint64_t pulse_id, const BufferBinaryFormat* buffer);

};


#endif //BINARYWRITER_HPP
