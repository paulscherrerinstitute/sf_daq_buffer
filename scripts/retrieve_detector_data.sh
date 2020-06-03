#!/bin/bash

if [ $# -lt 3 ]
then
    echo "Usage : $0 detector_name start_pulse_id end_pulse_id"
    echo "Example : $0 JF07T32V01 11709404000 11709405000"
    echo "Optional parameters:                                  output_file_name"
    exit
fi

DETECTOR=$1
START_PULSE_ID=$2
STOP_PULSE_ID=$3

echo "Request to retrieve : $@ "
echo "Started                 : "`date`
date1=$(date +%s)

if [ $# == 4 ]
then
    OUTFILE=$4
else
    OUTFILE=/gpfs/photonics/swissfel/buffer/test.${START_PULSE_ID}-${STOP_PULSE_ID}.h5
fi

case ${DETECTOR} in
'JF01T03V01')
  NM=3
  ;;
'JF07T32V01')
  NM=32
  ;;
'JF13T01V01')
  NM=1
  ;;
*)
  NM=1
esac

#8 replay workers per core
#coreAssociated_replay=(20 20 20 20 20 20 20 20 21 21 21 21 21 21 21 21 22 22 22 22 22 22 22 22 23 23 23 23 23 23 23 23)
#4 replay workers per core
coreAssociated_replay=(7 7 7 7 8 8 8 8 9 9 9 9 10 10 10 10 11 11 11 11 12 12 12 12 13 13 13 13 14 14 14 14)
#2 replay workers per core 
#coreAssociated_replay=(20 20 21 21 22 22 23 23 24 24 25 25 26 26 27 27 28 28 29 29 30 30 31 31 32 32 33 33 34 34 35 35)
#1 replay workers per core
#coreAssociated_replay=(4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35)

#coreAssociated="7,7,7,7,8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14"
coreAssociated="7,8,9,10,11,12,13,14"

touch /tmp/detector_retrieve.log /tmp/detector_retrieve_replay.log

cd /gpfs/photonics/swissfel/buffer/

PREVIOUS_STILL_RUN=1
while [ ${PREVIOUS_STILL_RUN} = 1 ]
do
    sleep 15 # we need to sleep at least to make sure that we don't read from CURRENT file
    PREVIOUS_STILL_RUN=0
#    ps -fe | grep "/usr/bin/sf_replay " | grep -v grep | grep sf_ > /dev/null
    PREVIOUS_STILL_RUN1=1
    ps -fe | grep "/usr/bin/sf_writer " | grep -v grep | grep sf_ > /dev/null 
    PREVIOUS_STILL_RUN2=$?
    if [ ${PREVIOUS_STILL_RUN1} != 1 -o ${PREVIOUS_STILL_RUN2} != 1 ]
    then
        PREVIOUS_STILL_RUN=1
#        echo "Previous retrieve is not yet finished ${PREVIOUS_STILL_RUN1} ${PREVIOUS_STILL_RUN2}"
#        sleep 30
    fi
done

date2=$(date +%s)
echo -n "Waited Time   : "
echo $((date2-date1)) | awk '{print int($1/60)":"int($1%60)}' 
echo "Started actual retrieve : "`date`

#for M in {00..31}
#do
#    taskset -c ${coreAssociated_replay[10#${M}]} /usr/bin/sf_replay ${PROCESS_PID} ${DETECTOR} M${M} ${M} ${START_PULSE_ID} ${STOP_PULSE_ID} >> /tmp/detector_retrieve_replay.log &
#done

taskset -c ${coreAssociated} /usr/bin/sf_writer ${OUTFILE} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${NM} ${START_PULSE_ID} ${STOP_PULSE_ID} >> /tmp/detector_retrieve.log &

wait

date3=$(date +%s)
echo "Finished                : "`date`
echo -n "Retrieve Time : "
echo $((date3-date2)) | awk '{print int($1/60)":"int($1%60)}'

