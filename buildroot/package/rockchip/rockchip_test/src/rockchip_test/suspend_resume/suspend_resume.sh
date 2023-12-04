#!/bin/sh

RESULT_DIR=/data/cfg/rockchip_test/
RESULT_LOG=${RESULT_DIR}/suspend_resume.log
SUSPEND_MAX=20
SUSPEND_MIN=5
SUSPEND_INTERVAL=$(($SUSPEND_MAX - $SUSPEND_MIN + 1 ))
WAKE_MAX=20
WAKE_MIN=5
WKAE_INTERVAL=$(($WAKE_MAX - $WAKE_MIN + 1 ))
MAX_CYCLES=5000

mkdir -p ${RESULT_DIR}

# suspend & resume test
echo "**************************************"
echo "auto suspend:                        1"
echo "suspend (resume by key):             2"
echo "auto suspend (resume by rtc):        3"
echo "**************************************"

read  SUSPEND_CHOICE

random() {
  hexdump -n 2 -e '/2 "%u"' /dev/urandom
}

suspend_resume_by_Key()
{
    echo "System will suspend, Please resume by key"
    pm-suspend
}

auto_suspend_resume()
{
    while true
    do
        pm-suspend
        sleep 5
    done
}

auto_suspend_resume_rtc()
{
    cnt=0
    while true; do
    echo "have done $cnt suspend/resume"
    if [ $cnt -ge $MAX_CYCLES ]
    then
        echo "run $MAX_CYCLES cycles, finish test"
        exit 0
    fi
    sus_time=$(( ( $(random) % $SUSPEND_INTERVAL ) + $SUSPEND_MIN ))
    echo "sleep for $sus_time second"
    echo 0 > /sys/class/rtc/rtc0/wakealarm
    echo "+${sus_time}" > /sys/class/rtc/rtc0/wakealarm
    pm-suspend
    wake_time=$(( ( $(random) % $WKAE_INTERVAL ) + $WAKE_MIN ))
    echo "wake for $wake_time second"
    sleep $wake_time
    let "cnt=$cnt+1"
    done
}

case ${SUSPEND_CHOICE} in
    1)
        auto_suspend_resume &
        ;;
    2)
        suspend_resume_by_Key
        ;;
    3)
        auto_suspend_resume_rtc
	;;
esac
