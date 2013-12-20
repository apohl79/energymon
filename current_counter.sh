#!/bin/bash
DIR=/opt/energymon
$DIR/impulse_counter.pl current 2>&1 >/var/log/energymon/current.log &
echo $! > /var/run/current_counter.pid
