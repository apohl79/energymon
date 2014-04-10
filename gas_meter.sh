#!/bin/bash
DIR=/opt/energymon
$DIR/impulse_counter.pl gas >/var/log/energymon/gas.log 2>&1 &
echo $! > /var/run/gas_meter.pid
