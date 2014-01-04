#!/bin/bash
DIR=/opt/energymon
$DIR/heating.py 2>&1 >/var/log/energymon/heating.log &
echo $! > /var/run/heating.pid
