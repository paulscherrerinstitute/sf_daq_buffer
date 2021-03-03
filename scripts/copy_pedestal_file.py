import argparse
import json
import os
import datetime
from shutil import copyfile

PEDESTAL_DIRECTORY="/sf/jungfrau/data/pedestal"

parser = argparse.ArgumentParser()

parser.add_argument("file_pedestal", type=str)
parser.add_argument("json_run", type=str)
parser.add_argument("detector", type=str)
parser.add_argument("json_stream", type=str)

args = parser.parse_args()

with open(args.json_run, "r") as run_file:
    data = json.load(run_file)

    request_time=datetime.datetime.strptime(data["request_time"], '%Y-%m-%d %H:%M:%S.%f')

    if not os.path.isdir(f'{PEDESTAL_DIRECTORY}/{args.detector}'):
        os.mkdir(f'{PEDESTAL_DIRECTORY}/{args.detector}')

    out_name = f'{PEDESTAL_DIRECTORY}/{args.detector}/{request_time.strftime("%Y%m%d_%H%M%S")}.h5'
    copyfile(args.file_pedestal, out_name)

    print(f'Copied resulting pedestal file {args.file_pedestal} to {out_name}')

    if not os.path.exists(args.json_stream):
        print(f'stream file {args.json_stream} does not exists, exiting')
        exit()

    with open(args.json_stream, "r") as stream_file:
        det = json.load(stream_file)

    print(f'Changing in stream file {args.json_stream} pedestal from {det["pedestal_file"]} to {out_name}')

    det["pedestal_file"] = out_name

    with open(args.json_stream, "w") as write_file:
        json.dump(det, write_file, indent=4)

