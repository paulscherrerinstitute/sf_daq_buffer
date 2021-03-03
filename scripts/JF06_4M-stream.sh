#!/bin/bash

coreAssociated="20,21"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF06_4M.daq8.json
SERVICE=JF06_4M-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}

