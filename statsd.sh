#!/bin/bash
DIR=/opt/statsd
/usr/bin/nodejs $DIR/stats.js $DIR/config.js >/var/log/statsd.log 2>&1 &
echo $! > /var/run/statsd.pid
