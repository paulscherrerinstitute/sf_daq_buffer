from argparse import ArgumentParser
from sf_utils.BinaryBufferReader import BinaryBufferReader

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
        is_good_frame = bool(input_is_good_frame[i][0])

        meta, data = reader.read_pulse_id(pulse_id)

        if pulse_id != meta["pulse_id"]:
            print("pulse_id mismatch",
                  "expected", pulse_id, "got", meta["pulse_id"])

        if frame_index != meta["frame_index"]:
            print("frame_index mismatch"
                  "expected", frame_index, "got", meta["frame_index"])

        if daq_rec != meta["daq_rec"]:
            print("daq_rec mismatch"
                  "expected", daq_rec, "got", meta["daq_rec"])

        if is_good_frame != meta["is_good_frame"]:
            print("is_good_frame mismatch"
                  "expected", is_good_frame, "got", meta["is_good_frame"])

        numpy.testing.assert_array_equal(input_data[i], data)

        print("pulse_id", pulse_id, "verification completed.")


if __name__ == "__main__":
    main()
