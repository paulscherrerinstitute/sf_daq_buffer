export PATH=/home/dbe/miniconda3/bin:$PATH

source /home/dbe/miniconda3/etc/profile.d/conda.sh

conda deactivate
conda activate vis

PORT=5003
PORT_BACKEND=9003

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
BACKEND=${H}

CORES=39

taskset -c ${CORES} \
streamvis bernina --allow-websocket-origin=${H}:${PORT} \
--allow-websocket-origin=sf-daq-bernina:${PORT} --port=${PORT} \
--address tcp://${BACKEND}:${PORT_BACKEND} \
--page-title I0

