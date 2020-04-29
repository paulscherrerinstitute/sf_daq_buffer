#ifndef WRITERUTILS_H
#define WRITERUTILS_H

#include <string>

namespace WriterUtils {
    void set_process_effective_id(int user_id);
    void create_destination_folder(const std::string& output_file);
}

#endif // WRITERUTILS_H
