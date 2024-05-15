#!/usr/bin/env python

import logging
from argparse import ArgumentParser
from ctypes import c_byte, c_char, c_uint64, sizeof, Structure

import numpy


logging.basicConfig()
_logger = logging.getLogger(__name__)


FOLDER_MOD = 100000
FILE_MOD = 1000
FILE_EXTENSION = ".bin"
MODULE_X_SIZE = 1024
MODULE_Y_SIZE = 512
BYTES_PER_PIXEL = 2
MODULE_N_PIXELS = MODULE_X_SIZE * MODULE_Y_SIZE
MODULE_N_BYTES = MODULE_N_PIXELS * BYTES_PER_PIXEL


class BufferBinaryFormat(Structure):
    _pack_ = 1
    _fields_ = [
        ("FORMAT_MARKER",  c_char),
        ("pulse_id",       c_uint64),
        ("frame_index",    c_uint64),
        ("daq_rec",        c_uint64),
        ("n_recv_packets", c_uint64),
        ("module_id",      c_uint64),
        ("data",           c_byte * MODULE_N_BYTES)
    ]


BUFFER_BINARY_SIZE = sizeof(BufferBinaryFormat)



class BinaryBufferReader:

    def __init__(self, root_folder, n_modules):
        self.root_folder = root_folder
        self.n_modules = n_modules


    def read_pulse_id(self, pulse_id):
        index_in_file = get_file_frame_index(pulse_id)
        n_bytes_offset = int(index_in_file * BUFFER_BINARY_SIZE)
        n_bytes_to_read = BUFFER_BINARY_SIZE

        metadata = {
            "pulse_id": 0,
            "frame_index": 0,
            "daq_rec": 0,
            "is_good_frame": True
        }

        shape = [self.n_modules * MODULE_N_BYTES]
        data = numpy.zeros(shape=shape, dtype="byte")

        metadata_init = False

        for i_module in range(self.n_modules):
            output_prefix = f"[pulse_id {pulse_id} module {i_module}]"

            device_name = f"M{i_module:02}"
            filename = get_filename(self.root_folder, device_name, pulse_id)

            with open(filename, "rb") as input_file:
                input_file.seek(n_bytes_offset)
                input_data = input_file.read(n_bytes_to_read)
                frame_buffer = BufferBinaryFormat.from_buffer_copy(input_data)

            if frame_buffer.FORMAT_MARKER != b"\xBE":
                _logger.warning(f"{output_prefix} no data in buffer")
                metadata["is_good_frame"] = False
                continue

            start_byte_image = MODULE_N_BYTES * i_module
            stop_byte_image = start_byte_image + MODULE_N_BYTES

            frame_data = numpy.array(frame_buffer.data, dtype="bytes")
            data[start_byte_image:stop_byte_image] = frame_data

            is_good_frame = (frame_buffer.n_recv_packets == 128)

            if not is_good_frame:
                n_lost_packets = 128 - frame_buffer.n_recv_packets
                _logger.warning(f"{output_prefix} n_lost_packets: {n_lost_packets}")
                metadata["is_good_frame"] = False
                continue

            current = {
                "pulse_id":    frame_buffer.pulse_id,
                "frame_index": frame_buffer.frame_index,
                "daq_rec":     frame_buffer.daq_rec
            }

            if not metadata_init:
                metadata.update(current)
                metadata_init = True

            if metadata["is_good_frame"]:
                for key, val in current.items():
                    md_val = metadata[key]
                    if val != md_val:
                        _logger.warning(f"{output_prefix} Mismatch {key}: {val} != {md_val}")
                        metadata["is_good_frame"] = False

        if not metadata_init:
            metadata["is_good_frame"] = False

        data_shape = [self.n_modules * MODULE_Y_SIZE, MODULE_X_SIZE]
        data = data.view("uint16").reshape(data_shape)
        return metadata, data



def get_file_frame_index(pulse_id):
    file_base = int((pulse_id // FILE_MOD) * FILE_MOD)
    return pulse_id - file_base

def get_filename(root_folder, device_name, pulse_id):
    folder_base = int((pulse_id // FOLDER_MOD) * FOLDER_MOD)
    file_base = int((pulse_id // FILE_MOD) * FILE_MOD)
    return f"{root_folder}/{device_name}/{folder_base}/{file_base}{FILE_EXTENSION}"





def main():
    parser = ArgumentParser(description="Read DAQ Binary Buffer")

    parser.add_argument("root_folder", type=str, help="absolute path to root folder of device")
    parser.add_argument("n_modules",   type=int, help="number of modules to read from this device")
    parser.add_argument("pulse_id",    type=int, help="pulse ID to retrieve")
    parser.add_argument("--log_level", default="WARNING", choices=["CRITICAL", "ERROR", "WARNING", "INFO", "DEBUG"], help="log level")

    clargs = parser.parse_args()

    _logger.setLevel(clargs.log_level)

    reader = BinaryBufferReader(clargs.root_folder, clargs.n_modules)

    metadata, _data = reader.read_pulse_id(clargs.pulse_id)
    print(metadata)



if __name__ == "__main__":
    main()



