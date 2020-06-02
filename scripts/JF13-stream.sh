#!/bin/bash

coreAssociated="25"

taskset -c ${coreAssociated} /usr/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF13.json
