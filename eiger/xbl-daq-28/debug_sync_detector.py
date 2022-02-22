#!/usr/bin/env python
from slsdet import Eiger
n_test = 1000
n_frames = 1000
d = Eiger()
d.nextframenumber = 1
d.frames = n_frames
d.period = 0.001
d.exptime = 0.0005

for i in range(0,n_test):
    d.acquire()
    next_fn = d.nextframenumber
    print(next_fn)
    if not isinstance(next_fn, int):
        raise ValueError(f'Frame numbers differ: {next_fn}')
