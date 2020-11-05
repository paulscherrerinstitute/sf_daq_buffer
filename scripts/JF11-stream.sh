#!/bin/bash

coreAssociated="14,15,16"

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF11.json
