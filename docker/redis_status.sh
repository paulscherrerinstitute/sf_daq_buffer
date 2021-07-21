#!/bin/sh
set -e

if [[ -z "${REDIS_STATUS_KEY}" ]]; then
  echo "Environment variable REDIS_STATUS_KEY not defined."
  exit 1;
fi

STATUS="$(redis-cli -x set "${REDIS_STATUS_KEY}:config" < redis_config.json)"
if [ ! "${STATUS}" = "OK" ]; then
  echo "Cound not set service status in Redis: ${STATUS}"
  exit 1;
fi

while true; do
  TIMESTAMP="$(date +%s%N)"

  STATUS="$(redis-cli set "${REDIS_STATUS_KEY}:heartbeat" ${TIMESTAMP} )"
  if [ ! "${STATUS}" = "OK" ]; then
    echo "Cound not set service hearbeat in Redis: ${STATUS}"
    exit 1;
  fi

  # Update heartbeat every 10 seconds.
  sleep 10
done
