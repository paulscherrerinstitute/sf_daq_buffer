#!/bin/bash

coreAssociated="17,18,19"

taskset -c ${coreAssociated} /usr/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF02.json
