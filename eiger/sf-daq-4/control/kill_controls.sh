#!/bin/bash
UDP=0
SYNC=0
ASSEMBLER=0
WRITER=0
HELP_FLAG=0
while getopts h:u:s:a:w: flag
do
    case "${flag}" in
        h ) HELP_FLAG=${OPTARG};;
        u ) UDP=${OPTARG};;
        s ) SYNC=${OPTARG};;
        w ) WRITER=${OPTARG};;
        a ) ASSEMBLER=${OPTARG};;
    esac
done

if (( ${UDP} == 0 )) && (( ${SYNC} == 0 )) && (( ${ASSEMBLER} == 0 )) && (( ${WRITER} == 0 )); then
    echo "Nothing to do..."
    exit
fi

# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -u <udp_recvs>  -s <stream> -a <assembler> -h <help_flag>"
    echo "           udp : kill udp receivers."
    echo "           sync : kill sync."
    echo "           assembler : kill assembler."
    echo "           writer : kill writer."
    echo "           help_flag : show this help and exits."
    exit
fi

if (( ${UDP} == 1 )); then
    echo "Killing upd recvs..."
    ps aux | grep std_udp_recv | awk 'NR > 1 { print prev } { prev = $2 }' | xargs -I{} kill {}
fi

if (( ${SYNC} == 1 )); then
    echo "Killing sync..."
    ps aux | grep std_udp_sync | awk 'NR > 1 { print prev } { prev = $2 }' | xargs -I{} kill {}
fi

if (( ${ASSEMBLER} == 1 )); then
    echo "Killing assembler..."
    ps aux | grep eiger_assembler | awk 'NR > 1 { print prev } { prev = $2 }' | xargs -I{} kill {}
fi

if (( ${WRITER} == 1 )); then
    echo "Killing writer..."
    ps aux | grep mpiexec | awk 'NR > 1 { print prev } { prev = $2 }' | xargs -I{} kill {}
fi


