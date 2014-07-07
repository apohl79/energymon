#!/bin/bash
DIR=/opt/statsd
#LOGF=/var/log/statsd.log
LOGF=/dev/null
/usr/bin/nodejs $DIR/stats.js $DIR/config.js >$LOGF 2>&1 &
echo $! > /var/run/statsd.pid
