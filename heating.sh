#!/bin/bash
DIR=/opt/energymon
$DIR/heating.py >/var/log/energymon/heating.log 2>&1 &
echo $! > /var/run/heating.pid
