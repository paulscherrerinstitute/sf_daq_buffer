#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF13-buffer-worker@00
    exit
fi

M=$1

# Add ourselves to the user cpuset.
# echo $$ > /sys/fs/cgroup/cpuset/user/tasks

coreAssociatedBuffer=(13)

initialUDPport=50190
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF13T01V01
N_MODULES=1

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer ${DETECTOR} ${N_MODULES} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
