#!/bin/bash

coreAssociated="24"

taskset -c ${coreAssociated} /usr/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF01.json
