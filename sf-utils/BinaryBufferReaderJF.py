#!/usr/bin/env python

import logging
from argparse import ArgumentParser
from glob import glob

from BinaryBufferReader import BinaryBufferReader


logging.basicConfig()
_logger = logging.getLogger("BinaryBufferReader")


BUFFER_DIR = "/gpfs/photonics/swissfel/buffer"


def get_root_folder(detector):
    pattern = f"{BUFFER_DIR}/{detector}*"
    dns = glob(pattern)
    nfound = len(dns)
    if nfound == 1:
        return dns[0]
    if nfound == 0:
        raise SystemExit(f"no root folder matching {detector}")
    if nfound > 1:
        raise SystemExit(f"cannot unambiguously matched {detector} to root folder: {dns}")

def get_pulse_id(root_folder):
    latest = f"{root_folder}/M00/LATEST"
    with open(latest) as f:
        data = f.read()
    #/gpfs/photonics/swissfel/buffer/JF02T09V03/M00/21125800000/21125820000.bin
    pid = data.strip().split("/")[-1].split(".")[0]
    return int(pid)

def get_n_modules(root_folder):
    detname = get_detname(root_folder)
    nmod = int(detname[5:7])
    return nmod

def get_detname(root_folder):
    return root_folder.split("/")[-1]





def main():
    parser = ArgumentParser(description="Read DAQ Binary Buffer for JFs")

    parser.add_argument("detector", help="detector name (JF01...)")
    parser.add_argument("--n_modules", type=int, help="number of modules to read from this device")
    parser.add_argument("--pulse_id", type=int, help="pulse ID to retrieve")
    parser.add_argument("--log_level", default="WARNING", choices=["CRITICAL", "ERROR", "WARNING", "INFO", "DEBUG"], help="log level")

    clargs = parser.parse_args()

    detector = clargs.detector

    root_folder = get_root_folder(detector)
    pulse_id = clargs.pulse_id or get_pulse_id(root_folder)
    n_modules = clargs.n_modules or get_n_modules(root_folder)

    _logger.setLevel(clargs.log_level)

    reader = BinaryBufferReader(root_folder, n_modules)

    metadata, _data = reader.read_pulse_id(pulse_id)
    print(metadata)



if __name__ == "__main__":
    main()



