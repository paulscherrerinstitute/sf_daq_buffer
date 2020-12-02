#!/bin/bash

coreAssociated="37"

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF09.json
