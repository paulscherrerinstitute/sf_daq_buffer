#!/bin/bash

coreAssociated="26"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF10.json
SERVICE=JF10-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}
