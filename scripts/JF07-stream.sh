#!/bin/bash

coreAssociated="20,21,22,23"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF07.json
SERVICE=JF07-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}
