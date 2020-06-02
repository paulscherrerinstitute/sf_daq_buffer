#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF01-buffer-worker@{00..02}
    exit
fi

M=$1

# Add ourselves to the user cpuset.
# echo $$ > /sys/fs/cgroup/cpuset/user/tasks

coreAssociatedBuffer=(12 12 12)

initialUDPport=50010
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF01T03V01

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
