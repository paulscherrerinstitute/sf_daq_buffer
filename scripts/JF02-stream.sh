#!/bin/bash

coreAssociated="9,10,11,12"

taskset -c ${coreAssociated} /usr/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF02.json
