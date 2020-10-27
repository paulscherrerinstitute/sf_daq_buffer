import argparse

import numpy as np
import os

import h5py
import json

def is_it_dark(laser_mode, detector_rate, pulseid):

    dark_rate = 1
    if 100/detector_rate == int(100/detector_rate):
        dark_rate = 100/detector_rate
        if laser_mode == 11: # 50/50 mode
            dark_rate *= 2
        elif laser_mode == 41: # 4 lights, 1 dark sequence
            dark_rate *= 5
        elif laser_mode == 111: # 11 lights, 1 dark sequence
            dark_rate *= 12
        elif laser_mode == 191: # 19 lights, 1 dark sequence
            dark_rate *=20

    dark = True

    if laser_mode == 0:
        dark = True
    elif laser_mode == 1:
        dark = False
    elif laser_mode == 13:
        if (pulseid % int(100/detector_rate*4)) == 0:
            dark = False
    else:
        if (pulseid + int(100/detector_rate) ) % dark_rate == 0:
            dark = True
        else:
            dark = False

    return dark

def which_dark(laser_mode, detector_rate, pulseid):

    dark_mode = -1
    if laser_mode != 13:
        dark_mode = 0
    else:
        for m in range(1,4):
            if ((pulseid-m*int(100/detector_rate)) % int(100/detector_rate*4)) == 0:
                dark_mode = m

    return dark_mode

parser = argparse.ArgumentParser()
parser.add_argument("data_file", type=str)
parser.add_argument("run_info", type=str)
parser.add_argument("detector", type=str)
args = parser.parse_args()

data_file = args.data_file
run_info_file = args.run_info
detector = args.detector

try:
    with open(run_info_file) as json_file:
        parameters = json.load(json_file)
except:
    print("Can't read provided run file {run_info_file}, may be not json?")
    exit()

laser_mode = parameters.get("laser_mode", 0)
rate_multiplicator = parameters.get("rate_multiplicator", 1)
detector_rate = 100//rate_multiplicator

print("Laser mode: ", laser_mode, ", detector runs at ", detector_rate, "Hz")

try:
    f=h5py.File(data_file, "r")
except:
    print(f"Can't open {data_file}")
    exit()

pulseids = f[f'/data/{detector}/pulse_id'][:]
n_pulse_id = len(pulseids)
if f'/data/{detector}/is_good_frame' in f.keys():
    is_good_frame = f[f'/data/{detector}/is_good_frame'][:]
else:
    is_good_frame = [1] * n_pulse_id

nGoodFrames = 0
nProcessedFrames = 0

index_dark = []
index_light = []

index_dark_mode = {}

for i in range(len(pulseids)):
    if not is_good_frame[i]:
        continue
    nGoodFrames += 1
    p = pulseids[i]
    nProcessedFrames += 1
    if is_it_dark(laser_mode, detector_rate, p):
        index_dark.append(i)
        if laser_mode == 13:
            dark_mode = which_dark(laser_mode, detector_rate, p)
            if dark_mode not in index_dark_mode:
                index_dark_mode[dark_mode] = []
            index_dark_mode[dark_mode].append(i) 
    else:
        index_light.append(i)

f.close()

print("Total number of frames: %s, number of good frames : %s, processed frames: %s, outputed frames: %s(dark) %s(light) " % (len(pulseids), nGoodFrames, nProcessedFrames, len(index_dark), len(index_light)) )

delim = '//'

if len(index_dark) > 0:
    file_dark = data_file[:-3] + ".dark.lst"
    if laser_mode == -1:
        file_dark = data_file[:-3] + ".undefined.lst"
    print(f"List of dark frames : {file_dark} , {len(index_dark)} frames")
    f_list = open(file_dark, "w")
    for frame_number in index_dark:
        print(f'{data_file} //{frame_number}', file = f_list)
    f_list.close()

if len(index_light) > 0:
    file_light = data_file[:-3] + ".light.lst"
    print(f"List of light frames : {file_light} , {len(index_light)} frames")
    f_list = open(file_light, "w")
    for frame_number in index_light:
        print(f'{data_file} {delim}{frame_number}', file = f_list)
    f_list.close()


for m in index_dark_mode:
    if len(index_dark_mode[m]) > 0:
        file_dark = f'{data_file[:-3]}.dark{m}.lst'
        print(f"List of dark{m} frames : {file_dark} , {len(index_dark_mode[m])} frames")
        f_list = open(file_dark, "w")
        for frame_number in index_dark_mode[m]:
            print(f'{data_file} //{frame_number}', file = f_list)
        f_list.close()

