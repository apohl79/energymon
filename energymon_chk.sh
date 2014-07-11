#!/bin/bash
if [ $(ps -ef|egrep "(heating.py|impulse_counter.pl (gas|power))"|grep -v grep|wc -l) -lt 3 ]; then
    export PATH=$PATH:/usr/sbin/:/usr/bin/:/sbin:/bin
    service energymon restart
fi
