#!/bin/bash

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
case ${H} in
'sf-daq-4')
    coreAssociated="33,34,35"
    config=/gpfs/photonics/swissfel/buffer/config/stream-JF02.json
    ;;
'sf-daq-8')
    coreAssociated="20,21"
    config=/gpfs/photonics/swissfel/buffer/config/stream-JF02.daq8.json
    ;;
*)
    coreAssociated="12"
esac

taskset -c ${coreAssociated} /usr/local/bin/sf_stream ${config}
