#!/bin/bash

coreAssociated="25"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF09.json
SERVICE=JF09-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}
