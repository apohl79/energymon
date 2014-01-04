#!/bin/bash
DIR=/opt/energymon
$DIR/impulse_counter.pl gas 2>&1 >/var/log/energymon/gas.log &
echo $! > /var/run/gas_meter.pid
