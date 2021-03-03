#!/bin/bash

coreAssociated="24"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF01.json
SERVICE=JF01-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}
