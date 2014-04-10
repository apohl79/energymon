#!/bin/bash
DIR=/opt/energymon
LOG=/var/log/energymon
mkdir -p $LOG 2>/dev/null
$DIR/impulse_counter.pl gas >$LOG/gas.log 2>&1 &
echo $! > /var/run/gas_meter.pid
