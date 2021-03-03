export PATH=/home/dbe/miniconda3/bin:$PATH

source /home/dbe/miniconda3/etc/profile.d/conda.sh

conda deactivate
conda activate vis

PORT=5006
PORT_BACKEND=9006

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
BACKEND=${H}

CORES="35,36"

taskset -c ${CORES} \
streamvis default16m --allow-websocket-origin=${H}:${PORT} --allow-websocket-origin=sf-daq-alvra:${PORT} \
--port=${PORT} --address  tcp://${BACKEND}:${PORT_BACKEND} \
--page-title 16M_Jungfrau_Alvra

