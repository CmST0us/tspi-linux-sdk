#!/bin/bash
echo "start watchdog test"
echo "press keyboard ctrl+c to stop feeding watchdog"
echo "press keyboard ctrl+c to stop feeding watchdog"
echo "press keyboard ctrl+c to stop feeding watchdog"

trap 'onCtrlC' INT
function onCtrlC () {
    echo ""
    echo "CTRL+C signal detected, watchdog feeding is stopped, and system will restart after 10s"
    exit 0
}

/rp_test/watchdog/watchdogd.out 1 10 
