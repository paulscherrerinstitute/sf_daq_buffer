#!/bin/bash

if [ $# -lt 3 ]
then
    echo "Usage :   $0 detector_name start_pulse_id end_pulse_id "
    echo "Example : $0 JF07T32V01    11709404000    11709405000  "
    echo "Optional parameters:                                   output_file_name rate_multiplicator"
    exit
fi

DETECTOR=$1
START_PULSE_ID=$2
STOP_PULSE_ID=$3
PULSE_ID_STEP=1 # by default assume 100Hz

echo "Request to retrieve : $@ "
echo "Started                 : "`date`
date1=$(date +%s)

if [ $# -ge 4 ]
then
    OUTFILE=$4
else
    OUTFILE=/gpfs/photonics/swissfel/buffer/test.${START_PULSE_ID}-${STOP_PULSE_ID}.h5
fi

if [ $# -eq 5 ]
then
    PULSE_ID_STEP=$5
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

coreAssociated="7,8,9,10,11,12,13,14"

touch /tmp/detector_retrieve.log

cd /gpfs/photonics/swissfel/buffer/

PREVIOUS_STILL_RUN=0
while [ ${PREVIOUS_STILL_RUN} == 0 ]
do
    sleep 15 # we need to sleep at least to make sure that we don't read from CURRENT file
    ps -fe | grep "bin/sf_writer " | grep -v grep | grep sf_writer > /dev/null 
    PREVIOUS_STILL_RUN=$? # not found == 1
done

date2=$(date +%s)
echo -n "Waited Time   : "
echo $((date2-date1)) | awk '{print int($1/60)":"int($1%60)}' 
echo "Started actual retrieve : "`date`

taskset -c ${coreAssociated} /usr/bin/sf_writer ${OUTFILE} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${NM} ${START_PULSE_ID} ${STOP_PULSE_ID} ${PULSE_ID_STEP}>> /tmp/detector_retrieve.log &

wait

date3=$(date +%s)
echo "Finished                : "`date`
echo -n "Retrieve Time : "
echo $((date3-date2)) | awk '{print int($1/60)":"int($1%60)}'

