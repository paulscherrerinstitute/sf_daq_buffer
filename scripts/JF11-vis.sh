export PATH=/home/dbe/miniconda3/bin:$PATH

source /home/dbe/miniconda3/etc/profile.d/conda.sh

conda deactivate
conda activate vis

PORT=5011
PORT_BACKEND=9011

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
BACKEND=${H}

taskset -c 17,18 \
streamvis alvra --allow-websocket-origin=${H}:${PORT} \
--allow-websocket-origin=sf-daq-alvra:${PORT} --port=${PORT} \
--address tcp://${BACKEND}:${PORT_BACKEND} \
--page-title TXS_Flex
