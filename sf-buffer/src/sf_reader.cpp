#include <iostream>
#include <chrono>
#include <UdpReceiver.hpp>
#include <fstream>
#include <thread>
#include "jungfrau.hpp"
#include "BufferUtils.hpp"


using namespace std;

int main (int argc, char *argv[]) {
    if (argc != 3) {
        cout << endl;
        cout << "Usage: sf_reader [device_name] [root_folder]";
        cout << endl;
        cout << "\tdevice_name: Device files to read.";
        cout << "\troot_folder: FS root folder." << endl;
        cout << endl;

        exit(-1);
    }

    string device_name = string(argv[1]);
    string root_folder = string(argv[2]);

    string current_filename = root_folder + "/" + device_name + "/CURRENT";

    uint64_t pulse_id_buffer[1000];
    uint16_t* image_buffer = new uint16_t[100*512*1024];

    string last_open_file = "";
    uint64_t last_pulse_id = 0;

    int current_file_last_processed = -1;

    while (true) {
//        auto filename = BufferUtils::get_latest_file(current_filename);
//
//        // Next file not yet ready.
//        if (last_open_file == filename) {
//            this_thread::sleep_for(chrono::milliseconds(100));
//            cout << "Waiting for CURRENT to change." << endl;
//            continue;
//        }
//
//        std::cout << "Opening " << filename << endl;
//        last_open_file = filename;
//        current_file_last_processed = -1;
//
//        H5::H5File input_file(filename, H5F_ACC_RDONLY |  H5F_ACC_SWMR_READ);
//        auto image_dataset = input_file.openDataSet("image");
//        auto pulse_id_dataset = input_file.openDataSet("pulse_id");
//
//        ::memset(pulse_id_buffer, 0, sizeof(pulse_id_buffer));
//
//        while (true) {
//
//            pulse_id_dataset.read(
//                    pulse_id_buffer,
//                    H5::PredType::NATIVE_UINT64);
//
//            size_t n_new_pulses = 0;
//            for (size_t i=current_file_last_processed+1; i<1000; i++) {
//                if (pulse_id_buffer[i] > 0) {
//                    n_new_pulses++;
//                }
//            }
//
//            // There is more stuff to be processed.
//            if (n_new_pulses > 0) {
//                // TODO: Just temporary due to buffer size.
//                if (n_new_pulses > 100) {
//                    n_new_pulses = 100;
//                }
//
//                H5Drefresh(image_dataset.getId());
//
//                uint64_t start_pulse_id = current_file_last_processed+1;
//                uint64_t end_pulse_id =
//                        current_file_last_processed + n_new_pulses;
//
//
//                hsize_t buff_dim[3] = {100, 512, 1024};
//                H5::DataSpace buffer_space (3, buff_dim);
//                hsize_t b_count[] = {n_new_pulses, 512, 1024};
//                hsize_t b_start[] = {0, 0, 0};
//                buffer_space.selectHyperslab(H5S_SELECT_SET, b_count, b_start);
//
//                hsize_t disk_dim[3] = {1000, 512, 1024};
//                H5::DataSpace disk_space(3, disk_dim);
//
//                hsize_t d_count[] = {n_new_pulses, 512, 1024};
//                hsize_t d_start[] = {start_pulse_id, 0, 0};
//                disk_space.selectHyperslab(H5S_SELECT_SET, d_count, d_start);
//
//                image_dataset.read(
//                        image_buffer,
//                        H5::PredType::NATIVE_UINT16,
//                        buffer_space,
//                        disk_space);
//
//                current_file_last_processed = end_pulse_id;
//
//                cout << "Read n_new_pulses=" << n_new_pulses;
//                cout << " current_file_last_processed ";
//                cout << current_file_last_processed << endl;
//            }
//
//            // Time for next file.
//            if (pulse_id_buffer[999] != 0) {
//                break;
//            }
//
//            // Stream delay.
//            this_thread::sleep_for(chrono::milliseconds(100));
//            H5Drefresh(pulse_id_dataset.getId());
//        }

    }
}
