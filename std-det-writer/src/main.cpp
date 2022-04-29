#include <iostream>
#include <string>
#include <zmq.h>
#include <mpi.h>
#include <unistd.h>
#include <sstream>
#include <chrono>

#include "RamBuffer.hpp"
#include "BufferUtils.hpp"
#include "live_writer_config.hpp"
#include "WriterStats.hpp"
#include "JFH5Writer.hpp"
#include "DetWriterConfig.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "date.h"

using namespace std;
using namespace date;
using namespace buffer_config;
using namespace live_writer_config;

int main (int argc, char *argv[])
{
    if (argc != 2) {
        cout << endl;
        cout << "Usage: std_det_writer [detector_json_filename]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << endl;

        exit(-1);
    }

    auto const config = DetWriterConfig::from_json_file(string(argv[1]));

    MPI_Init(nullptr, nullptr);

    int n_writers;
    MPI_Comm_size(MPI_COMM_WORLD, &n_writers);

    int i_writer;
    MPI_Comm_rank(MPI_COMM_WORLD, &i_writer);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, LIVE_ZMQ_IO_THREADS);
    auto receiver = BufferUtils::connect_socket_gf(
            ctx, config.detector_name, "tcp://localhost:9667");

    const size_t IMAGE_N_BYTES = config.image_width * config.image_height * config.bit_depth / 8;

    JFH5Writer writer(config.detector_name);
    WriterStats stats(config.detector_name, IMAGE_N_BYTES);

    char recv_buffer_meta[512];
    char recv_buffer_data[4838400];
    bool open_run = false;
    bool header_in = false;
    int last_run_id = -1;
    while (true) {
        auto nbytes = zmq_recv(receiver, &recv_buffer_meta, sizeof(recv_buffer_meta), 0);
        rapidjson::Document document;
        if (document.Parse(recv_buffer_meta, nbytes).HasParseError()) {
            std::string error_str(recv_buffer_meta, nbytes);
            throw runtime_error(error_str);
        }
        
        const string output_file = document["output_file"].GetString();
        const uint64_t image_id = document["image_id"].GetUint64();
        const int run_id = document["run_id"].GetInt();
        const int i_image = document["i_image"].GetInt();
        const int n_images = document["n_images"].GetInt();
        const int user_id = document["user_id"].GetInt();




        const int status = document["status"].GetInt();
        const rapidjson::Value& a = document["shape"];
        const int width = a[0].GetInt();
        const int heigth = a[1].GetInt();
        const int dtype = 2;


        // i_image == n_images -> end of run.
        if (i_image == n_images && open_run == true) {
            writer.close_run();
            stats.end_run();
	    open_run = false;

            #ifdef DEBUG_OUTPUT 
                cout << "[" << std::chrono::system_clock::now() << "]";
                cout << "[std_daq_det_writer] Setting real group/user id..." << endl;
            #endif
            if (setresgid(0,0,0)){
                stringstream error_message;
                cout << " Problem setting real group id..." << endl;
                error_message << "[" << std::chrono::system_clock::now() << "]";
                error_message << "[std_daq_det_writer] Cannot set real group id..." << endl;
                throw runtime_error(error_message.str());
            }
            if (setresuid(0,0,0)){
                stringstream error_message;
                error_message << "[" << std::chrono::system_clock::now() << "]";
                error_message << "[std_daq_det_writer] Cannot set real user id..." << endl;
                throw runtime_error(error_message.str());
            }
            continue;
        }

        // i_image == 0 -> we have a new run.
        if (i_image == 0 && open_run == false) {
            // TODO Improve changing GID and UID of the writer processes 
            // to be part of the deployment via the ansible deployment.
            #ifdef DEBUG_OUTPUT
                 cout << "[" << std::chrono::system_clock::now() << "]";
                 cout << "[std_daq_det_writer] Setting process uid to " << user_id << endl;
            #endif
            
            if (setegid(user_id)) {
                stringstream error_message;
                error_message << "[" << std::chrono::system_clock::now() << "]";
                error_message << "[std_daq_det_writer] Cannot set group_id to " << user_id << endl;
                throw runtime_error(error_message.str());
            }

            if (seteuid(user_id)) {
                stringstream error_message;
                error_message << "[" << std::chrono::system_clock::now() << "]";
                error_message << "[std_daq_det_writer] Cannot set user_id to " << user_id << endl;
                throw runtime_error(error_message.str());
            }

            writer.open_run(output_file,
                            run_id,
                            n_images,
                            heigth,
                            width,
                            dtype);
            open_run = true;
	}


        // data
        auto img_nbytes = zmq_recv(receiver, &recv_buffer_data, sizeof(recv_buffer_data), 0);
        if (img_nbytes != -1 && header_in == true && open_run == true){
            // Fair distribution of images among writers.
            if (i_image % n_writers == i_writer) {
                stats.start_image_write();
                writer.write_data(run_id, i_image, recv_buffer_data);
                stats.end_image_write();
            }
            header_in = false;
        }
	
	
        // Only the first instance writes metadata.
        if (i_writer == 0 && header_in == true && open_run == true) {
            writer.write_meta_gf(run_id, i_image, 
                    (uint16_t)run_id, 
                    (uint64_t)status);
            }

        }
    
}
