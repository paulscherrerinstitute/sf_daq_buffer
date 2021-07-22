#!/bin/bash
set -e

echo "Starting Redis and broker.."
docker-compose up -d broker redis
sleep 1

echo "Setting test daq_profile config to Redis.."
redis-cli -x set config.debug.test_pipeline < docker/example_detector.json

echo "Starting pipeline.."
docker-compose up -d