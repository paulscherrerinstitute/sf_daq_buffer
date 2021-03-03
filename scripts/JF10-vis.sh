export PATH=/home/dbe/miniconda3/bin:$PATH

source /home/dbe/miniconda3/etc/profile.d/conda.sh

conda deactivate
conda activate vis

PORT=5010
PORT_BACKEND=9010

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
BACKEND=${H}

CORES=38

taskset -c ${CORES} \
streamvis alvra --allow-websocket-origin=${H}:${PORT} \
--allow-websocket-origin=sf-daq-alvra:${PORT} --port=${PORT} \
--address tcp://${BACKEND}:${PORT_BACKEND} \
--page-title FLEX:Stripsel

