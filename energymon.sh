#!/bin/bash
DIR=/opt/energymon
$DIR/energymon.pl >/var/log/energymon/energymon.log 2>&1 &
echo $! > /var/run/energymon.pid
