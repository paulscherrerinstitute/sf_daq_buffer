#!/bin/bash

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
case ${H} in
'sf-daq-4')
    coreAssociated="33,34,35"
    CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF02.daq4.json
    ;;
'sf-daq-8')
    coreAssociated="20,21"
    CONFIG=/gpfs/photonics/swissfel/buffer/config/stream-JF02.json
    ;;
*)
    coreAssociated="12"
esac

SERVICE=JF02-stream

/home/dbe/git/sf_daq_buffer/scripts/check_config_changed.sh ${CONFIG} ${SERVICE} &

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${CONFIG}

