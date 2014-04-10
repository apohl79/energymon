#!/bin/bash
DIR=/opt/energymon
$DIR/impulse_counter.pl power >/var/log/energymon/power.log 2>&1 &
echo $! > /var/run/power_meter.pid
