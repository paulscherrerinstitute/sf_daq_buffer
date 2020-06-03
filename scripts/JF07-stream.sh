#!/bin/bash

coreAssociated="20,21,22,23"

taskset -c ${coreAssociated} /usr/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF07.json
