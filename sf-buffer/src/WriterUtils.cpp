#include <iostream>
#include <unistd.h>
#include <filesystem>

#include "WriterUtils.hpp"
#include "date.h"

using namespace std;

void WriterUtils::set_process_effective_id(int user_id)
{

    // TODO: use setfsuid and setfsgid

    if (setegid(user_id)) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[WriterUtils::set_process_effective_id]";
        err_msg << " Cannot set group_id to " << user_id << endl;

        throw runtime_error(err_msg.str());
    }

    if (seteuid(user_id)) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[WriterUtils::set_process_effective_id]";
        err_msg << " Cannot set user_id to " << user_id << endl;

        throw runtime_error(err_msg.str());
    }
}

void WriterUtils::create_destination_folder(const string& output_file)
{
    auto file_separator_index = output_file.rfind('/');

    if (file_separator_index != string::npos) {

        string output_folder(output_file.substr(0, file_separator_index));

        string create_folder_command("mkdir -p " + output_folder);
        system(create_folder_command.c_str());
    }
}
