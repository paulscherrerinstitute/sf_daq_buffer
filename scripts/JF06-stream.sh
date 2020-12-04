#!/bin/bash

coreAssociated="22,23,24"

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF06.daq8.json
