from argparse import ArgumentParser
from BinaryBufferReader import BinaryBufferReader

import numpy
import h5py

def main():
    parser = ArgumentParser(description='Verify final H5 file against buffer.')

    parser.add_argument('h5_filename', type=str,
                        help="Input file to verify.")

    parser.add_argument('--base_folder', type=str,
                        default="/gpfs/photonics/swissfel/buffer",
                        help="Absolute path to root folder of device.")

    arguments = parser.parse_args()

    file = h5py.File(arguments.h5_filename, 'r')
    detector_name = list(file['data'])[0]
    n_modules = file["/data/" + detector_name + "/data"][0].shape[0] // 512

    reader = BinaryBufferReader(arguments.base_folder + "/" + detector_name,
                                n_modules)

    input_data = file["/data/" + detector_name + "/data"]
    input_pulse_id = file["/data/" + detector_name + "/pulse_id"]
    input_frame_index = file["/data/" + detector_name + "/frame_index"]
    input_daq_rec = file["/data/" + detector_name + "/daq_rec"]
    input_is_good_frame = file["/data/" + detector_name + "/is_good_frame"]

    for i in range(input_pulse_id.shape[0]):
        pulse_id = input_pulse_id[i][0]
        frame_index = input_frame_index[i][0]
        daq_rec = input_daq_rec[i][0]
        is_good_frame = input_is_good_frame[i][0]

        meta, data = reader.read_pulse_id(pulse_id)

        print(pulse_id, meta.pulse_id)
        print(frame_index, meta.frame_index)
        print(daq_rec, meta.daq_rec)
        print(is_good_frame, meta.is_good_frame)


if __name__ == "__main__":
    main()