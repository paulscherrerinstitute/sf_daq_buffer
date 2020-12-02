#!/bin/bash

coreAssociated="38"

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF10.json
