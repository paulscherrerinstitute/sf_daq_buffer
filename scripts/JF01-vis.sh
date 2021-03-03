export PATH=/home/dbe/miniconda3/bin:$PATH

source /home/dbe/miniconda3/etc/profile.d/conda.sh

conda deactivate
conda activate vis

PORT=5001
PORT_BACKEND=9001

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
BACKEND=${H}

taskset -c 17,18 \
streamvis bernina --allow-websocket-origin=${H}:${PORT} \
--allow-websocket-origin=sf-daq-bernina:${PORT} --port=${PORT} \
--address tcp://${BACKEND}:${PORT_BACKEND} \
--page-title 1p5M

