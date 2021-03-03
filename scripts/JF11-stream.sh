#!/bin/bash

coreAssociated="14,15,16"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF11.json
SERVICE=JF11-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}
