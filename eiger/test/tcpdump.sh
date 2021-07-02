#!/bin/bash
INTERFACES=("$@")
echo $# interfaces will be analysed:
# array of proc ids
declare -a PROC_IDS=()
# start tcpdump for the specified interfaces
for i in "${INTERFACES[@]}"
do
    if [ -f "$i".dump ]; then
        echo "Removing file $i.dump"
        rm "$i".dump
    fi
    if [ -f "$i".txt ]; then
        echo "Removing file $i.txt"
        rm $i.txt
    fi
    # starts the tcpdump for each interface
    echo "Starting tcpdump on $i..."
    #echo $i
    nohup tcpdump -i $i -enn -B 400000000 -w $i.dump &
    PROC_IDS+=($!)
done

# loop waiting to stop
echo "Press ESC key to quit"
# read a single character
while read -r -n1 key
do
# if input == ESC key
if [[ $key == $'\e' ]];
then
    break;
fi
done

# kills tcp dump processes
for i in "${PROC_IDS[@]}"
do
    echo Killing proccess with pid $i
    kill $i
done
# delete if previous report file exists
report_filename='tcp_dump_report.txt'
if [ -f $report_filename ]; then
    rm $report_filename
    # creates an empty file
    touch $report_filename
fi
touch $report_filename
# converts raw to text and parses the file
for i in "${INTERFACES[@]}"
do
    echo "Treating raw data into txt file and parsing relevant info..."
    tcpdump -r $i.dump > $i.txt && cat $i.txt | awk '{print $5" "$8}' | awk -F. '{print $4}' | awk -F: '{print $1" "$2}' | sort | uniq -c >> $report_filename && sed '/length/d' -i $report_filename
done

echo "Finishing tcpdump analysis..."