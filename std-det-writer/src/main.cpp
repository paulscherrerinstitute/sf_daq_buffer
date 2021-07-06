#include <iostream>
#include <string>
#include <zmq.h>
#include <mpi.h>

#include "RamBuffer.hpp"
#include "BufferUtils.hpp"
#include "live_writer_config.hpp"
#include "WriterStats.hpp"
#include "broker_format.hpp"
#include "JFH5Writer.hpp"
#include "DetWriterConfig.hpp"

using namespace std;
using namespace buffer_config;
using namespace live_writer_config;

int main (int argc, char *argv[])
{
    if (argc != 2) {
        cout << endl;
        cout << "Usage: std-det-writer [detector_json_filename]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << endl;

        exit(-1);
    }

    auto const config = DetWriterConfig::from_json_file(string(argv[1]));

    MPI_Init(nullptr, nullptr);

    int n_writers;
    MPI_Comm_size(MPI_COMM_WORLD, &n_writers);

    int i_writer;
    MPI_Comm_size(MPI_COMM_WORLD, &i_writer);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, LIVE_ZMQ_IO_THREADS);
    auto receiver = BufferUtils::connect_socket(
            ctx, config.detector_name, "writer_agent");

    const size_t IMAGE_N_BYTES = 12;
    RamBuffer image_buffer(config.detector_name + "_assembler",
            sizeof(ImageMetadata), IMAGE_N_BYTES, 1, RAM_BUFFER_N_SLOTS);

    JFH5Writer writer(config.detector_name);
    WriterStats stats(config.detector_name);

    StoreStream meta = {};
    while (true) {
        zmq_recv(receiver, &meta, sizeof(meta), 0);

        // i_image == 0 -> we have a new run.
        if (meta.i_image == 0) {
            auto image_meta = (ImageMetadata*)
                    image_buffer.get_slot_meta(meta.image_id);

            writer.open_run(meta.output_file,
                            meta.run_id,
                            meta.n_images,
                            image_meta->height,
                            image_meta->width,
                            image_meta->dtype);

            stats.start_run(meta);
        }

        // i_image == n_images -> end of run.
        if (meta.i_image == meta.n_images) {
            writer.close_run();

            stats.end_run();
            continue;
        }

        // Fair distribution of images among writers.
        if (meta.i_image % n_writers == i_writer) {
            char* data = image_buffer.get_slot_data(meta.image_id);

            stats.start_image_write();
            writer.write_data(meta.run_id, meta.i_image, data);
            stats.end_image_write();
        }

        // Only the first instance writes metadata.
        if (i_writer == 0) {
            auto image_meta = (ImageMetadata*)
                    image_buffer.get_slot_meta(meta.image_id);
            writer.write_meta(meta.run_id, meta.i_image, image_meta);
        }

    }
}
