#!/bin/bash
DIR=/opt/energymon
LOGD=/var/log/energymon

CMD_HEATING=heating.py
CMD_GAS="impulse_counter.pl gas"
CMD_POWER="impulse_counter.pl power"

case "$1" in
    "heating")
        CMD=$DIR/$CMD_HEATING
        ;;
    "gas")
        CMD=$DIR/$CMD_GAS
        ;;
    "power")
        CMD=$DIR/$CMD_POWER
        ;;
    *)
        echo "Usage: $0 {heating|gas|power} [debug]"
        exit 1
        ;;
esac

LOGF=$LOGD/$1.err

if [ "$2" == "debug" ]; then
    LOGDBG=$LOGD/$1.log
else
    LOGDBG=/dev/null
fi

mkdir -p $LOGD 2>/dev/null

if [ "$2" == "debug" ]; then
    echo
    echo "exec: $CMD >$LOGDBG 2>$LOGF &"
fi

$CMD >$LOGDBG 2>$LOGF &
PID=$!
echo $PID > /var/run/$1.pid

if [ "$2" == "debug" ]; then
    echo "pid: $PID"
fi

