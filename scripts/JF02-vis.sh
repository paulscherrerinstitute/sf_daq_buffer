export PATH=/home/dbe/miniconda3/bin:$PATH

source /home/dbe/miniconda3/etc/profile.d/conda.sh

conda deactivate
conda activate vis

PORT=5002
PORT_BACKEND=9002

H=`echo ${HOSTNAME} | sed 's/.psi.ch//'`
BACKEND=${H}


case ${H} in
'sf-daq-4')
  CORES='36,37'
  ;;
'sf-daq-8')
  CORES='33,34'
  ;;
*)
  CORES='2'
esac

taskset -c ${CORES} \
streamvis alvra --allow-websocket-origin=${H}:${PORT} \
--allow-websocket-origin=sf-daq-alvra:${PORT} --port=${PORT} \
--address tcp://${BACKEND}:${PORT_BACKEND} \
--page-title 4p5M_Alvra

