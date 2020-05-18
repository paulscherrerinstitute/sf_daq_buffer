#!/bin/bash

if [ $# != 3 ]
then
    echo "Usage : $0 detector_name start_pulse_id end_pulse_id"
    echo "Example : $0 JF07T32V01 11709404000 11709405000"
    exit
fi

DETECTOR=$1
START_PULSE_ID=$2
STOP_PULSE_ID=$3

#8 replay workers per core, last (writer) worker occupies 4
#coreAssociated_replay=(20 20 20 20 20 20 20 20 21 21 21 21 21 21 21 21 22 22 22 22 22 22 22 22 23 23 23 23 23 23 23 23)
#4 replay workers per core, last (writer) worker occupies 4
#coreAssociated_replay=(20 20 20 20 21 21 21 21 22 22 22 22 23 23 23 23 24 24 24 24 25 25 25 25 26 26 26 26 27 27 27 27)
#2 replay workers per core, last (writer) worker occupies 4
#coreAssociated_replay=(20 20 21 21 22 22 23 23 24 24 25 25 26 26 27 27 28 28 29 29 30 30 31 31 32 32 33 33 34 34 35 35)
#1 replay workers per core, last (writer) worker occupies 4
coreAssociated_replay=(8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39)


coreAssociated_writer="24,25,26,27"

#latest_file=`cat /gpfs/photonics/swissfel/buffer/JF07T32V01/M00/LATEST`
#last_pulse_id=`basename ${latest_file} | sed 's/.h5//'`
#first_pulse_id=$((${last_pulse_id}-360000))

#echo "First/last pulse_id : ${first_pulse_id} ${last_pulse_id}"

touch /tmp/detector_retrieve.log

for M in {00..31}
do
    taskset -c ${coreAssociated_replay[10#${M}]} /usr/bin/sf_replay ${DETECTOR} M${M} ${M} ${START_PULSE_ID} ${STOP_PULSE_ID} >> /tmp/detector_retrieve.log &
done

taskset -c ${coreAssociated_writer} /usr/bin/sf_writer /gpfs/photonics/swissfel/buffer/test.${START_PULSE_ID}-${STOP_PULSE_ID}.h5 ${START_PULSE_ID} ${STOP_PULSE_ID} >> /tmp/detector_retrieve.log &

wait

