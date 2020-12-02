export PATH=/home/dbe/miniconda3/bin:$PATH

source /home/dbe/miniconda3/etc/profile.d/conda.sh

conda deactivate
conda activate vis

PORT=5004
PORT_BACKEND=9004

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
BACKEND=${H}

taskset -c 16 \
streamvis bernina --allow-websocket-origin=${H}:${PORT} \
--allow-websocket-origin=sf-daq-bernina:${PORT} --port=${PORT} \
--address tcp://${BACKEND}:${PORT_BACKEND} \
--page-title Fluorescence

