#include <iostream>
#include <string>
#include <zmq.h>
#include <RamBuffer.hpp>
#include <BufferUtils.hpp>
#include "live_writer_config.hpp"
#include "WriterStats.hpp"
#include "broker_format.hpp"
#include <mpi.h>
#include <JFH5Writer.hpp>


using namespace std;
using namespace buffer_config;
using namespace live_writer_config;

int main (int argc, char *argv[])
{
    if (argc != 3) {
        cout << endl;
        cout << "Usage: jf_live_writer [detector_json_filename]"
                " [bits_per_pixel]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tbits_per_pixel: Number of bits in each pixel." << endl;
        cout << endl;

        exit(-1);
    }

    auto const config = BufferUtils::read_json_config(string(argv[1]));
    auto const bits_per_pixel = atoi(argv[2]);

    MPI_Init(NULL, NULL);

    int n_writers;
    MPI_Comm_size(MPI_COMM_WORLD, &n_writers);

    int i_writer;
    MPI_Comm_size(MPI_COMM_WORLD, &i_writer);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, LIVE_ZMQ_IO_THREADS);
    auto receiver = BufferUtils::connect_socket(
            ctx, config.detector_name, "broker-agent");

    RamBuffer ram_buffer(config.detector_name, config.n_modules);

    const uint64_t image_n_bytes =
            config.image_y_size * config.image_x_size * bits_per_pixel;

    JFH5Writer writer(config);
    WriterStats stats(config.detector_name, STATS_MODULO, image_n_bytes);

    StoreStream meta = {};
    while (true) {
        zmq_recv(receiver, &meta, sizeof(meta), 0);

        if (meta.op_code == OP_START) {
            writer.open_run(meta.run_id, meta.n_images);
            continue;
        }

        if (meta.op_code == OP_END) {
            writer.close_run();
            continue;
        }

        // Fair distribution of images among writers.
        if (meta.i_image % n_writers == i_writer) {
            char* data = ram_buffer.read_image(meta.image_metadata.pulse_id);

            stats.start_image_write();
            writer.write_data(meta.run_id, meta.i_image, data);
            stats.end_image_write();
        }

        // Only the first instance writes metadata.
        if (i_writer == 0) {
            writer.write_meta(meta.run_id, meta.i_image, meta.image_metadata);
        }
    }

    MPI_Finalize();
}
