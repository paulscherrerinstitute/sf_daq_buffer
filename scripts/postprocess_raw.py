import os
import struct

import bitshuffle
import h5py
import numpy as np
from bitshuffle.h5 import H5_COMPRESS_LZ4, H5FILTER  # pylint: disable=no-name-in-module

# bitshuffle hdf5 filter params
BLOCK_SIZE = 2048
compargs = {"compression": H5FILTER, "compression_opts": (BLOCK_SIZE, H5_COMPRESS_LZ4)}
# limit bitshuffle omp to a single thread
# a better fix would be to use bitshuffle compiled without omp support
os.environ["OMP_NUM_THREADS"] = "1"

DTYPE = np.dtype(np.uint16)
DTYPE_SIZE = DTYPE.itemsize

MODULE_SIZE_X = 1024
MODULE_SIZE_Y = 512


def postprocess_raw(
    source, dest, disabled_modules=(), index=None, compression=False, batch_size=100
):
    # a function for 'visititems' should have the args (name, object)
    def _visititems(name, obj):
        if isinstance(obj, h5py.Group):
            h5_dest.create_group(name)

        elif isinstance(obj, h5py.Dataset):
            dset_source = h5_source[name]

            # process all but the raw data
            if name != data_dset:
                if name.startswith("data"):
                    # datasets with data per image, so indexing should be applied
                    if index is None:
                        data = dset_source[:]
                    else:
                        data = dset_source[index, :]

                    args = {"shape": data.shape}
                    h5_dest.create_dataset_like(name, dset_source, data=data, **args)
                else:
                    h5_dest.create_dataset_like(name, dset_source, data=dset_source)

        else:
            raise TypeError(f"Unknown h5py object type {obj}")

        # copy group/dataset attributes if it's not a dataset with the actual data
        if name != data_dset:
            for key, value in h5_source[name].attrs.items():
                h5_dest[name].attrs[key] = value

    with h5py.File(source, "r") as h5_source, h5py.File(dest, "w") as h5_dest:
        detector_name = h5_source["general/detector_name"][()].decode()
        data_dset = f"data/{detector_name}/data"

        # traverse the source file and copy/index all datasets, except the raw data
        h5_source.visititems(_visititems)

        # now process the raw data
        dset = h5_source[data_dset]

        args = dict()
        if index is None:
            n_images = dset.shape[0]
        else:
            index = np.array(index)
            n_images = len(index)

        n_modules = dset.shape[1] // MODULE_SIZE_Y
        out_shape = (MODULE_SIZE_Y * (n_modules - len(disabled_modules)), MODULE_SIZE_X)

        args["shape"] = (n_images, *out_shape)
        args["maxshape"] = (n_images, *out_shape)
        args["chunks"] = (1, *out_shape)

        if compression:
            args.update(compargs)

        h5_dest.create_dataset_like(data_dset, dset, **args)

        # calculate and save module_map
        module_map = []
        tmp = 0
        for ind in range(n_modules):
            if ind in disabled_modules:
                module_map.append(-1)
            else:
                module_map.append(tmp)
                tmp += 1

        h5_dest[f"data/{detector_name}/module_map"] = np.tile(module_map, (n_images, 1))

        # prepare buffers to be reused for every batch
        read_buffer = np.empty((batch_size, *dset.shape[1:]), dtype=DTYPE)
        out_buffer = np.zeros((batch_size, *out_shape), dtype=DTYPE)

        # process and write data in batches
        for batch_start_ind in range(0, n_images, batch_size):
            batch_range = range(batch_start_ind, min(batch_start_ind + batch_size, n_images))

            if index is None:
                batch_ind = np.array(batch_range)
            else:
                batch_ind = index[batch_range]

            # TODO: avoid unnecessary buffers
            read_buffer_view = read_buffer[: len(batch_ind)]
            out_buffer_view = out_buffer[: len(batch_ind)]

            # Avoid a stride-bottleneck, see https://github.com/h5py/h5py/issues/977
            if np.sum(np.diff(batch_ind)) == len(batch_ind) - 1:
                # consecutive index values
                dset.read_direct(read_buffer_view, source_sel=np.s_[batch_ind])
            else:
                for i, j in enumerate(batch_ind):
                    dset.read_direct(read_buffer_view, source_sel=np.s_[j], dest_sel=np.s_[i])

            for i, m in enumerate(module_map):
                if m == -1:
                    continue

                read_slice = read_buffer_view[:, i * MODULE_SIZE_Y : (i + 1) * MODULE_SIZE_Y, :]
                out_slice = out_buffer_view[:, m * MODULE_SIZE_Y : (m + 1) * MODULE_SIZE_Y, :]
                out_slice[:] = read_slice

            bytes_num_elem = struct.pack(">q", out_shape[0] * out_shape[1] * DTYPE_SIZE)
            bytes_block_size = struct.pack(">i", BLOCK_SIZE * DTYPE_SIZE)
            header = bytes_num_elem + bytes_block_size

            for pos, im in zip(batch_range, out_buffer_view):
                if compression:
                    byte_array = header + bitshuffle.compress_lz4(im, BLOCK_SIZE).tobytes()
                else:
                    byte_array = im.tobytes()

                h5_dest[data_dset].id.write_direct_chunk((pos, 0, 0), byte_array)
