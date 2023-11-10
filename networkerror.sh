#!/bin/bash

test_hosts="fritz mneme mqtt"
has_network_error=0

function is_host_up() {
    [ -n "$(ping -qc1 -W1 $1|grep "1 received")" ]
}

while ((1)); do
    all_hosts_up=1
    for h in $test_hosts; do
        if ! is_host_up $h; then
            echo "$h is down"
            has_network_error=1
            all_hosts_up=0
        fi
    done
    if [ $all_hosts_up -eq 1 ] && [ $has_network_error -eq 1 ]; then
        sleep 30#
        has_network_error=0
        echo "restarting services"
        service supervisor stop
        service supervisor start
    fi
    sleep 1
done
