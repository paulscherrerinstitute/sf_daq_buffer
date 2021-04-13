#!/bin/bash
UDP=0
STREAM=0
ASSEMBLER=0
HELP_FLAG=0
while getopts h:u:s:a: flag
do
    case "${flag}" in
        h ) HELP_FLAG=${OPTARG};;
        u ) UDP=${OPTARG};;
        s ) STREAM=${OPTARG};;
        a ) ASSEMBLER=${OPTARG};;
    esac
done

if (( ${UDP} == 0 )) && (( ${STREAM} == 0 )) && (( ${ASSEMBLER} == 0 )); then
    echo "Nothing to do..."
    exit
fi

# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -u <udp_recvs>  -s <stream> -a <assembler> -h <help_flag>"
    echo "           udp : kill udp receivers."
    echo "           stream : kill stream."
    echo "           assembler : kill assembler."
    echo "           help_flag : show this help and exits."
    exit
fi

if (( ${UDP} == 1 )); then
    echo "Killing upd recvs..."
    ps aux | grep eiger_udp_recv | awk 'NR > 1 { print prev } { prev = $2 }' | xargs -I{} kill {}
fi

if (( ${STREAM} == 1 )); then
    echo "Killing stream..."
    ps aux | grep eiger_stream | awk 'NR > 1 { print prev } { prev = $2 }' | xargs -I{} kill {}
fi

if (( ${ASSEMBLER} == 1 )); then
    echo "Killing assembler..."
    ps aux | grep eiger_assembler | awk 'NR > 1 { print prev } { prev = $2 }' | xargs -I{} kill {}
fi


