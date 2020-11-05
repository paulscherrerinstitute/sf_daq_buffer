#!/bin/bash

coreAssociated="2,3,4,5"

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF06_4M.daq8.json
