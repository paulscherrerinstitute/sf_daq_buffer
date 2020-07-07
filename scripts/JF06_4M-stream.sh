#!/bin/bash

coreAssociated="13,14,15,16"

taskset -c ${coreAssociated} /usr/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF06_4M.json
