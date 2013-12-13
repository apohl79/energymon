#!/bin/bash
DIR=/opt/energymon
$DIR/gas_counter.pl 2>&1 >/var/log/energymon/gas.log &
echo $! > /var/run/gas_counter.pid
