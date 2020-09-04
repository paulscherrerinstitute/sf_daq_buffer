#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF02-buffer-worker@{00..08}
    exit
fi

M=$1

# Add ourselves to the user cpuset.
# echo $$ > /sys/fs/cgroup/cpuset/user/tasks

coreAssociatedBuffer=(39 39 39 40 40 40 41 41 41)

initialUDPport=50020
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF02T09V02
N_MODULES=9

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer ${DETECTOR} ${N_MODULES} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
