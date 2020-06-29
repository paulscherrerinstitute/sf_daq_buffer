import numpy
from argparse import ArgumentParser
from ctypes import *

FOLDER_MOD = 100000
FILE_MOD = 1000
FILE_EXTENSION = ".bin"
MODULE_X_SIZE = 1024
MODULE_Y_SIZE = 512
MODULE_N_PIXELS = MODULE_X_SIZE * MODULE_Y_SIZE
MODULE_N_BYTES = MODULE_N_PIXELS * 2


class BufferBinaryFormat(Structure):
    _pack_ = 1
    _fields_ = [("FORMAT_MARKER", c_char),
                ("pulse_id", c_uint64),
                ("frame_index", c_uint64),
                ("daq_rec", c_uint64),
                ("n_recv_packets", c_uint64),
                ("module_id", c_uint64),
                ("data", c_byte * MODULE_N_BYTES)]


class BinaryBufferReader(object):
    def __init__(self, root_folder, n_modules):
        self.root_folder = root_folder
        self.n_modules = n_modules

    def read_pulse_id(self, pulse_id):

        index_in_file = get_file_frame_index(pulse_id)
        n_bytes_offset = int(index_in_file * sizeof(BufferBinaryFormat))
        n_bytes_to_read = sizeof(BufferBinaryFormat)

        metadata = {"pulse_id": 0,
                    "frame_index": 0,
                    "daq_rec": 0,
                    "is_good_frame": True}

        data = numpy.zeros(shape=[self.n_modules * MODULE_N_BYTES],
                           dtype="byte")

        metadata_init = False

        for i_module in range(self.n_modules):
            device_name = "M%02d" % i_module

            filename = get_filename(
                root_folder=self.root_folder,
                device_name=device_name,
                pulse_id=pulse_id)

            with open(filename, 'rb') as input_file:
                input_file.seek(n_bytes_offset)
                frame_buffer = BufferBinaryFormat.from_buffer_copy(
                    input_file.read(n_bytes_to_read))

            if frame_buffer.FORMAT_MARKER == b'\xBE':
                is_good_frame = frame_buffer.n_recv_packets == 128

                if is_good_frame:
                    if not metadata_init:
                        metadata["pulse_id"] = frame_buffer.pulse_id
                        metadata["frame_index"] = frame_buffer.frame_index
                        metadata["daq_rec"] = frame_buffer.daq_rec

                        metadata_init = True

                    if metadata["is_good_frame"]:
                        if metadata["pulse_id"] != frame_buffer.pulse_id:
                            print("pulse_id mismatch in module", i_module,
                                  "expected", metadata["pulse_id"],
                                  "got", frame_buffer.pulse_id)

                            metadata["is_good_frame"] = False

                        if metadata["frame_index"] != frame_buffer.frame_index:
                            print("frame_index mismatch in module", i_module,
                                  "expected", metadata["frame_index"],
                                  "got", frame_buffer.frame_index)

                            metadata["is_good_frame"] = False

                        if metadata["daq_rec"] != frame_buffer.daq_rec:
                            print("daq_rec mismatch in module", i_module,
                                  "expected", metadata["daq_rec"],
                                  "got", frame_buffer.daq_rec)

                            metadata["is_good_frame"] = False
                else:
                    metadata["is_good_frame"] = False
                    print("Module", i_module,
                          "n_lost_packets",  128-frame_buffer.n_recv_packets)

                start_byte_image = MODULE_N_BYTES * i_module
                stop_byte_image = start_byte_image + MODULE_N_BYTES

                frame_data = numpy.array(frame_buffer.data, dtype="bytes")
                data[start_byte_image:stop_byte_image] = frame_data

            else:
                metadata["is_good_frame"] = False
                print("No data for module", i_module, "at pulse_id", pulse_id)

        if not metadata_init:
            metadata["is_good_frame"] = False

        data_shape = [self.n_modules * MODULE_Y_SIZE, MODULE_X_SIZE]
        return metadata, data.view("uint16").reshape(data_shape)


def get_file_frame_index(pulse_id):
    file_base = int((pulse_id // FILE_MOD) * FILE_MOD)
    return pulse_id - file_base


def get_filename(root_folder, device_name, pulse_id):
    folder_base = int((pulse_id // FOLDER_MOD) * FOLDER_MOD)
    file_base = int((pulse_id // FILE_MOD) * FILE_MOD)

    return "%s/%s/%s/%s%s" % (root_folder,
                              device_name,
                              folder_base,
                              file_base,
                              FILE_EXTENSION)


def main():
    parser = ArgumentParser(description='Read DAQ Binary Buffer')
    parser.add_argument('root_folder', type=str,
                        help="Absolute path to root folder of device.")
    parser.add_argument('n_modules', type=int,
                        help="Number of modules to read from this device.")
    parser.add_argument('pulse_id', type=int,
                        help="Pulse_id to retrieve.")

    arguments = parser.parse_args()

    reader = BinaryBufferReader(arguments.root_folder,
                                arguments.n_modules)

    metadata, data = reader.read_pulse_id(arguments.pulse_id)
    print(metadata)


if __name__ == "__main__":
    main()
