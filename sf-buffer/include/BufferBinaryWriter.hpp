#ifndef BINARYWRITER_HPP
#define BINARYWRITER_HPP

#include <string>

#include "formats.hpp"

class BufferBinaryWriter {

    const size_t MAX_FILE_BYTES =
            buffer_config::FILE_MOD * sizeof(BufferBinaryFormat);

    const std::string detector_folder_;
    const std::string module_name_;
    std::string latest_filename_;

    std::string current_output_filename_;
    int output_file_fd_;

    void open_file(const std::string& filename);
    void close_current_file();


public:
    BufferBinaryWriter(
            const std::string& detector_folder,
            const std::string& module_name);

    virtual ~BufferBinaryWriter();

    void write(const uint64_t pulse_id, const BufferBinaryFormat* buffer);

};


#endif //BINARYWRITER_HPP
