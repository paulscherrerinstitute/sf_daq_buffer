#include <iostream>
#include <string>
#include <zmq.h>
#include <mpi.h>

#include "RamBuffer.hpp"
#include "BufferUtils.hpp"
#include "live_writer_config.hpp"
#include "WriterStats.hpp"
#include "JFH5Writer.hpp"
#include "DetWriterConfig.hpp"

#include "rapidjson/document.h"

using namespace std;
using namespace buffer_config;
using namespace live_writer_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        cout << "Usage: std_det_writer [detector_json_filename]"
                " [bit_depth]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tbit_depth: bit depth of the incoming udp packets." << endl;
        cout << endl;

        exit(-1);
    }

    auto const config = DetWriterConfig::from_json_file(string(argv[1]));
    const int bit_depth = atoi(argv[2]);

    MPI_Init(nullptr, nullptr);

    int n_writers;
    MPI_Comm_size(MPI_COMM_WORLD, &n_writers);

    int i_writer;
    MPI_Comm_rank(MPI_COMM_WORLD, &i_writer);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, LIVE_ZMQ_IO_THREADS);
    auto receiver = BufferUtils::connect_socket(
            ctx, config.detector_name, "writer_agent");

    const size_t IMAGE_N_BYTES = config.image_width * config.image_height * bit_depth / 8;
    RamBuffer image_buffer(config.detector_name + "_assembler",
            sizeof(ImageMetadata), IMAGE_N_BYTES, 1, RAM_BUFFER_N_SLOTS);

    JFH5Writer writer(config.detector_name);
    WriterStats stats(config.detector_name, IMAGE_N_BYTES);

    char recv_buffer[8192];
    while (true) {
        auto nbytes = zmq_recv(receiver, &recv_buffer, sizeof(recv_buffer), 0);
        rapidjson::Document document;
        if (document.Parse(recv_buffer, nbytes).HasParseError()) {
            std::string error_str(recv_buffer, nbytes);
            throw runtime_error(error_str);
        }
        
        const string output_file = document["output_file"].GetString();
        const uint64_t image_id = document["image_id"].GetUint64();
        const int run_id = document["run_id"].GetInt();
        const int i_image = document["i_image"].GetInt();
        const int n_images = document["n_images"].GetInt();
        // i_image == n_images -> end of run.
        if (i_image == n_images) {
            writer.close_run();
            stats.end_run();
            continue;
        }

        // i_image == 0 -> we have a new run.
        if (i_image == 0) {
            auto image_meta = (ImageMetadata*)
                image_buffer.get_slot_meta(image_id);

            writer.open_run(output_file,
                            run_id,
                            n_images,
                            image_meta->height,
                            image_meta->width,
                            image_meta->dtype);
        }


        // Fair distribution of images among writers.
        if (i_image % n_writers == i_writer) {
            char* data = image_buffer.get_slot_data(image_id);

            stats.start_image_write();
            writer.write_data(run_id, i_image, data);
            stats.end_image_write();
        }

        // Only the first instance writes metadata.
        if (i_writer == 0) {
            auto image_meta = (ImageMetadata*)
                image_buffer.get_slot_meta(image_id);
            writer.write_meta(run_id, i_image, image_meta);
        }

    }
}
