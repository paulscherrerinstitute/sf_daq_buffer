#!/bin/bash

GREP="std|streamvis"
while getopts g: flag
do
    case "${flag}" in
        g) GREP=${OPTARG};;
    esac
done
systemctl list-units --type service --all | grep -E ${GREP}  | awk 'BEGIN{print "Unit State Status"};$4 ~ /^running$/{print $1,$2,$4}' | column -t
