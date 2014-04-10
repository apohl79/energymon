#!/bin/bash
DIR=/opt/energymon
LOG=/var/log/energymon
mkdir -p $LOG 2>/dev/null
$DIR/heating.py >$LOG/heating.log 2>&1 &
echo $! > /var/run/heating.pid
