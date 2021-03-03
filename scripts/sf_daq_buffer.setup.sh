#!/bin/bash

# needed, otherwise executing with Ansible won't work
# see: https://github.com/conda/conda/issues/7267
unset SUDO_UID SUDO_GID SUDO_USER

if [ ! -d /home/dbe/git ]; then
  echo "No git repo found, cloning it..."
  mkdir /home/dbe/git
fi

REPO=sf_daq_buffer
if [ ! -d /home/dbe/git/${REPO} ]; then
  cd /home/dbe/git && git clone https://github.com/paulscherrerinstitute/${REPO}.git

  source /opt/rh/devtoolset-9/enable
  cd /home/dbe/git/${REPO} && mkdir -p build && cd build/ && cmake3 .. && make
fi


