#!/bin/bash

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
case ${H} in
'sf-daq-4')
    coreAssociated="33,34,35"
    ;;
*)
    coreAssociated="12"
esac

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF02.json
