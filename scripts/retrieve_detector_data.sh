#!/bin/bash

if [ $# -lt 3 ]
then
    echo "Usage :   $0 detector_name start_pulse_id end_pulse_id "
    echo "Example : $0 JF07T32V01    11709404000    11709405000  "
    echo "Optional parameters:                                   output_file_name rate_multiplicator jf_conversion run_file raw_file"
    exit
fi

DETECTOR=$1
START_PULSE_ID=$2
STOP_PULSE_ID=$3
PULSE_ID_STEP=1 # by default assume 100Hz
JF_CONVERSION=0 # by default don't call ju_export
RUN_FILE=None
RAW_FILE=None

echo "Request to retrieve : $@ "
echo "Started                 : "`date`
date1=$(date +%s)

if [ $# -ge 4 ]
then
    OUTFILE=$4
else
    OUTFILE=/gpfs/photonics/swissfel/buffer/test.${START_PULSE_ID}-${STOP_PULSE_ID}.h5
fi

if [ $# -ge 5 ]
then
    PULSE_ID_STEP=$5
fi

if [ $# -ge 6 ]
then
    JF_CONVERSION=$6
    if [ $# -ge 7 ]
    then
        RUN_FILE=$7
    fi
    if [ $# -eq 8 ]
    then
        RAW_FILE=$8
    fi
fi


case ${DETECTOR} in
'JF01T03V01')
  NM=3
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF01.json
  ;;
'JF02T09V02')
  NM=9
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF02.json
  ;;
'JF04T01V01')
  NM=1
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF04.json
  ;;
'JF03T01V02')
  NM=1
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF03.json
  ;;
'JF06T32V02')
  NM=32
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF06.json
  ;;
'JF06T08V02')
  NM=8
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF06_4M.daq8.json
  ;;
'JF07T32V01')
  NM=32
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF07.json
  ;;
'JF09T01V01')
  NM=1
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF09.json
  ;;
'JF10T01V01')
  NM=1
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF10.json
  ;;
'JF13T01V01')
  NM=1
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF13.json
  ;;
'JF11T04V01')
  NM=4
  DET_CONFIG_FILE=/gpfs/photonics/swissfel/buffer/config/stream-JF11.json
  ;;
*)
  NM=1
esac

#coreAssociated="7,8,9,10,11,12,13,14"
coreAssociated="9,10,11,12,13,14,15,16,17"

touch /tmp/detector_retrieve.log

cd /gpfs/photonics/swissfel/buffer/

PREVIOUS_STILL_RUN=0
while [ ${PREVIOUS_STILL_RUN} == 0 ]
do
    sleep 15 # we need to sleep at least to make sure that we don't read from CURRENT file
    n=`ps -fe | grep "bin/sf_writer " | grep -v grep | grep sf_writer | wc -l`
    if [ ${n} -lt 9 ]
    then
        PREVIOUS_STILL_RUN=1
    fi
done

date2=$(date +%s)
echo -n "Waited Time   : "
echo $((date2-date1)) | awk '{print int($1/60)":"int($1%60)}' 
echo "Started actual retrieve : "`date`

if [ ${JF_CONVERSION} == 0 ]
then
    OUTFILE_RAW=${OUTFILE}
else
    if [ ${RAW_FILE} != "None" ]
    then
        OUTFILE_RAW=${RAW_FILE}
        D1=`dirname ${OUTFILE_RAW}`
        mkdir -p ${D1}
    else
        RUN_NUMBER=`basename ${RUN_FILE} | awk -F '.' '{print $1}'`
        D1=`dirname ${RUN_FILE}`
        D2=`dirname ${D1}`
        OUTFILE_RAW=${D2}/.raw/${RUN_NUMBER}.${DETECTOR}.h5
        mkdir -p ${D2}/.raw/
    fi
fi

taskset -c ${coreAssociated} /usr/local/bin/sf_writer ${OUTFILE_RAW} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${NM} ${START_PULSE_ID} ${STOP_PULSE_ID} ${PULSE_ID_STEP}>> /tmp/detector_retrieve.log &

wait

#coreAssociatedConversion="35,34,33,32,31,30,29,28,27"
coreAssociatedConversion="35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18"
#coreAssociatedConversion="26,25,24,23,22,21,20,19,18"
#TODO: calculate this number from coreAssociatedConversion
#export NUMBA_NUM_THREADS=18

#not clear why, but bitshuffle doesn't respect OMP_NUM_THREADS set in jungfrau_utils anymore, thus we set it here
export OMP_NUM_THREADS=1

date3=$(date +%s)
echo "Finished                : "`date`
echo -n "Retrieve Time : "
echo $((date3-date2)) | awk '{print int($1/60)":"int($1%60)}'

if [ ${JF_CONVERSION} == 0 ]
then
    echo "File is written in raw format, no compression"
    
    dir_name=`dirname ${OUTFILE_RAW}`
    base_name=`basename ${dir_name}`

    if [ ${base_name} == "JF_pedestals" ]
    then
        echo "Pedestal run will make conversion"

        export PATH=/home/dbe/miniconda3/bin:$PATH

        source /home/dbe/miniconda3/etc/profile.d/conda.sh

        conda deactivate
        conda activate sf-daq

        if [ ${DETECTOR} == "JF07T32V01" ]
        then
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --add_pixel_mask /sf/bernina/config/jungfrau/pixel_mask/JF07T32V01/pixel_mask_13_full.h5 
        elif [ ${DETECTOR} == "JF03T01V02" ]
        then
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --add_pixel_mask /sf/bernina/config/jungfrau/pixel_mask/JF03T01V02/pixel_mask_half_chip.h5  
#            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG
        elif [ ${DETECTOR} == "JF02T09V02" ]
        then
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --number_bad_modules=1 
        elif [ ${DETECTOR} == "JF06T08V02" ]
        then
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --add_pixel_mask /sf/alvra/config/jungfrau/pixel_mask/JF06T08V01/mask_2lines_module3.h5 
#        elif [ ${DETECTOR} == "JF06T32V02" ]
#        then
#            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --add_pixel_mask /sf/alvra/config/jungfrau/pixel_mask/JF06T32V02/mask_noise_in_28.h5 
        elif [ ${DETECTOR} == "JF13T01V01" ]
        then
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --add_pixel_mask /sf/bernina/config/jungfrau/pixel_mask/JF13T01V01/pixel_mask_bad_rb_22.09.2020.h5 
        elif [ ${DETECTOR} == "JF11T04V01" ]
        then
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --number_bad_modules=2 
        elif [ ${DETECTOR} == "JF10T01V01" ]
        then
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG --number_bad_modules=1
        else
            time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/jungfrau_create_pedestals.py --filename ${OUTFILE_RAW} --directory ${dir_name} --verbosity DEBUG 
        fi

        PEDESTAL_FILE=`echo ${OUTFILE_RAW} | sed 's/.h5/.res.h5/'`

        taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/copy_pedestal_file.py ${PEDESTAL_FILE} ${RUN_FILE} ${DETECTOR} ${DET_CONFIG_FILE}

    fi

else
    echo "Will call compression/convertion ${OUTFILE_RAW} --> ${OUTFILE}"

    PREVIOUS_STILL_RUN=0
    while [ ${PREVIOUS_STILL_RUN} == 0 ]
    do
        sleep 15 # we need to sleep at least to make sure that we don't read from CURRENT file
        n=`ps -fe | grep "scripts/export_file.py " | grep -v grep | grep export | wc -l`
        if [ ${n} -lt 15 ]
        then
            PREVIOUS_STILL_RUN=1
        fi
    done
    date4=$(date +%s)
    echo -n "Sleep Time : "
    echo $((date4-date3)) | awk '{print int($1/60)":"int($1%60)}'

    export PATH=/home/dbe/miniconda3/bin:$PATH

    source /home/dbe/miniconda3/etc/profile.d/conda.sh

    conda deactivate
    conda activate sf-daq

    time taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/export_file.py ${OUTFILE_RAW} ${OUTFILE} ${RUN_FILE} ${DET_CONFIG_FILE}
    if [ ${DETECTOR} == "JF06T32V02" ] || [ ${DETECTOR} == "JF06T08V02" ]
    then
        taskset -c ${coreAssociatedConversion} python /home/dbe/git/sf_daq_buffer/scripts/make_crystfel_list.py ${OUTFILE} ${RUN_FILE} ${DETECTOR}
    fi
    date5=$(date +%s)
    echo "Finished                : "`date`
    echo -n "Conversion Time : "
    echo $((date5-date4)) | awk '{print int($1/60)":"int($1%60)}'

fi
