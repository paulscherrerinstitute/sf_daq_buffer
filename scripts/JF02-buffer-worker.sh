#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF02-buffer-worker@{00..08}
    exit
fi

M=$1

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`

case ${H} in
'sf-daq-4')
  coreAssociatedBuffer=(11 12 13 14 15 16 17 18 19)
  ;;
*)
  CORES=(25 25 26 26 27 27 28 28 29)
esac

initialUDPport=50020
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF02T09V02

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/local/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
