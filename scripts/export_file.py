import argparse
import json
import warnings

import h5py
import numpy as np

import jungfrau_utils as ju

parser = argparse.ArgumentParser()

parser.add_argument("file_in", type=str)
parser.add_argument("file_out", type=str)
parser.add_argument("json_run", type=str)
parser.add_argument("json_detector", type=str)

args = parser.parse_args()

with open(args.json_detector, "r") as detector_file:
    data = json.load(detector_file)

    detector_name = data["detector_name"]
    gain_file = data["gain_file"]
    pedestal_file = data["pedestal_file"]

with open(args.json_run, "r") as run_file:
    data = json.load(run_file)
    detector_params = data["detectors"][detector_name]

    compression = detector_params.get("compression", True)
    conversion = detector_params.get("adc_to_energy", True)
    if conversion:
        mask = detector_params.get("mask", True)
        mask_double_pixels = detector_params.get("mask_double_pixels", True)
        geometry = detector_params.get("geometry", False)
        gap_pixels = detector_params.get("gap_pixels", True)
        factor = detector_params.get("factor", None)
    else:
        mask = False
        mask_double_pixels = False
        geometry = False
        gap_pixels = False
        factor = None

if not mask and mask_double_pixels:
    warnings.warn("mask_double_pixels set to False")
    mask_double_pixels = False

with ju.File(
    args.file_in,
    gain_file=gain_file,
    pedestal_file=pedestal_file,
    conversion=conversion,
    mask=mask,
    gap_pixels=gap_pixels,
    geometry=geometry,
    parallel=True,
) as juf:
    n_input_frames = len(juf["data"])
    good_frames = np.nonzero(juf["is_good_frame"])[0]
    n_output_frames = len(good_frames)

    juf.handler.mask_double_pixels = mask_double_pixels
    juf.export(
        args.file_out,
        index=good_frames,
        roi=None,
        compression=compression,
        factor=factor,
        dtype=None,
        batch_size=35,
    )

    pixel_mask = juf.handler.get_pixel_mask(gap_pixels=gap_pixels, geometry=geometry)

# Postprocessing
with h5py.File(args.file_out, "r+") as h5f:
    h5f[f"/data/{detector_name}/pixel_mask"] = np.invert(pixel_mask)
    if conversion:
        print("daq_rec:", h5f[f"/data/{detector_name}/daq_rec"][0, 0])
        del h5f[f"/data/{detector_name}/daq_rec"]

        frame_index = h5f[f"/data/{detector_name}/frame_index"][:]
        print("frame_index range:", (np.min(frame_index), np.max(frame_index)))
        del h5f[f"/data/{detector_name}/frame_index"]

        del h5f[f"/data/{detector_name}/is_good_frame"]

print("input frames:", n_input_frames)
print("bad frames:", n_input_frames - n_output_frames)
print("output frames:", n_output_frames)

print("gain_file:", gain_file)
print("pedestal_file:", pedestal_file)
print("conversion:", conversion)
print("mask:", mask)
print("mask_double_pixels:", mask_double_pixels)
print("geometry:", geometry)
print("gap_pixels:", gap_pixels)
print("compression:", compression)
print("factor:", factor)

