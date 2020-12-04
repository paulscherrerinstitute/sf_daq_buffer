#!/bin/bash

coreAssociated="20,21"

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF06_4M.daq8.json
