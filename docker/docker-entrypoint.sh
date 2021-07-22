#!/bin/sh
set -e

if [[ -z "${PIPELINE_NAME}" ]]; then
  echo "Environment variable PIPELINE_NAME not defined."
  exit 1;
fi

if [[ -z "${SERVICE_NAME}" ]]; then
  echo "Environment variable SERVICE_NAME not defined."
  exit 1;
fi

REDIS_SKIP="${REDIS_SKIP:-false}"

# Download config from Redis to redis_config.json and start status reporting.
if [ "${REDIS_SKIP}" = false ]; then
  REDIS_HOST="${REDIS_HOST:-127.0.0.1}"
  REDIS_CONFIG_KEY=config."${PIPELINE_NAME}"
  REDIS_STATUS_KEY=status."${PIPELINE_NAME}.${SERVICE_NAME}"

  redis-cli -h "${REDIS_HOST}" get "${REDIS_CONFIG_KEY}" > redis_config.json

  CONFIG_BYTES="$(stat -c %s redis_config.json)"
  if [ "${CONFIG_BYTES}" -le 1 ]; then
      echo "Key missing in redis(${REDIS_HOST}): ${REDIS_CONFIG_KEY}"
      exit 1;
  fi

  export REDIS_STATUS_KEY
  redis_status.sh &
fi

EXECUTABLE="${@:1}"
exec "$EXECUTABLE"