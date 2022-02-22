#!/bin/bash

GREP="std|streamvis|journalbeat"
RESTART="OFF"
STATUS_VERBOSE="OFF"
while getopts g:r:s: flag
do
    case "${flag}" in
        g) GREP=${OPTARG};;
        r) RESTART=${OPTARG};;
        s) STATUS_VERBOSE=${OPTARG};;
    esac
done

if [ ${STATUS_VERBOSE} = "ON" ]; then
    systemctl list-units --type service --all | grep -E ${GREP} | awk '{print $1}' | xargs -I{} systemctl status {}
else
    systemctl list-units --type service --all | grep -E ${GREP} | awk 'BEGIN{print "Unit State Status"};$4 ~ /^running$/{print $1,$2,$4}'| column -t
fi

if [ ${RESTART} = "ON" ]; then
    systemctl list-units --type service --all | grep -E ${GREP} | awk '{print $1}' | xargs -I{} systemctl restart {}
fi 

#systemctl list-units --type service --all | grep -E ${GREP}  | awk 'BEGIN{print "Unit State Status"};$4 ~ /^running$/{print $1,$2,$4}' | column -t



