#!/bin/bash

coreAssociated="20,21,22,23"

taskset -c ${coreAssociated} /usr/bin/sf_stream tcp://129.129.241.42:9007 25 tcp://192.168.30.29:9107 10
