#!/bin/bash

coreAssociated="27"
CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF04.json
SERVICE=JF04-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}
