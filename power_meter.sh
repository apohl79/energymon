#!/bin/bash
DIR=/opt/energymon
$DIR/impulse_counter.pl power 2>&1 >/var/log/energymon/power.log &
echo $! > /var/run/power_meter.pid
