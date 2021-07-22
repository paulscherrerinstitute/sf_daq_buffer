#!/bin/sh
set -e

# How long the status record should be kept in the database.
EXPIRE_SECONDS=15
# At which interval should records be refreshed in the database.
STATUS_INTERVAL_SECONDS=10

if [[ -z "${REDIS_STATUS_KEY}" ]]; then
  echo "Environment variable REDIS_STATUS_KEY not defined."
  exit 1;
fi

STATUS="$(redis-cli -x hset "${REDIS_STATUS_KEY} config" < redis_config.json)"
if [ "${STATUS}" != 0 ] && [ "${STATUS}" != 1 ]; then
  echo "Cound not set service status in Redis: ${STATUS}"
  exit 1;
fi

STATUS="$(redis-cli expire "${REDIS_STATUS_KEY}" ${EXPIRE_SECONDS})"
if [ "${STATUS}" != 1 ]; then
  echo "Could not set status expire: ${STATUS}"
  exit 1;
fi

while true; do
  TIMESTAMP="$(date +%s%N)"

  STATUS="$(redis-cli hset "${REDIS_STATUS_KEY} heartbeat" "${TIMESTAMP}" )"
  if [ ! "${STATUS}" = "OK" ]; then
    echo "Cound not set service hearbeat in Redis: ${STATUS}"
    exit 1;
  fi

  STATUS="$(redis-cli expire "${REDIS_STATUS_KEY}" ${EXPIRE_SECONDS})"
  if [ "${STATUS}" != 1 ]; then
    echo "Could not set status expire: ${STATUS}"
    exit 1;
  fi

  # Update heartbeat every 10 seconds.
  sleep "${STATUS_INTERVAL_SECONDS}"
done
