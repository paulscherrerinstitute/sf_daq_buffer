#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF07-replay-worker@{00..32}
    exit
fi

M=$1

#8 replay workers per core, last (stream to visualisation) worker occupies 4
coreAssociated=(20 20 20 20 20 20 20 20 21 21 21 21 21 21 21 21 22 22 22 22 22 22 22 22 23 23 23 23 23 23 23 23 24,25,26,27)

latest_file=`cat /gpfs/photonics/swissfel/buffer/JF07T32V01/M00/LATEST`
last_pulse_id=`basename ${latest_file} | sed 's/.h5//'`
first_pulse_id=$((${last_pulse_id}-360000))

echo "First/last pulse_id : ${first_pulse_id} ${last_pulse_id}"

if [ ${M} == 32 ]
then
#    taskset -c ${coreAssociated[10#${M}]} /usr/bin/sf_writer /gpfs/photonics/swissfel/buffer/test.${first_pulse_id}-${last_pulse_id}.h5 ${first_pulse_id} ${last_pulse_id}
    taskset -c ${coreAssociated[10#${M}]} /usr/bin/sf_stream tcp://129.129.241.42:9007 30 tcp://192.168.30.29:9107 30
else
    taskset -c ${coreAssociated[10#${M}]} /usr/bin/sf_replay /gpfs/photonics/swissfel/buffer/JF07T32V01 M${M} ${M} ${first_pulse_id} ${last_pulse_id}
fi
