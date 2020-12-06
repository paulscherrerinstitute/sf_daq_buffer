#!/bin/bash

coreAssociated="22,23,24"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF06.json
SERVICE=JF06-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}
