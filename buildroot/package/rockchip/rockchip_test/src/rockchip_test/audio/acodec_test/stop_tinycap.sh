#!/bin/sh

#echo "Input process name first"
#read pname

echo "Stop tinycap process"
pname=tinycap

PID=$(ps -e|grep $pname|head -1|awk '{printf $1}')

if [ $? -eq 0 ]; then
    echo "process id:$PID"
else
    echo "process $pname not exit"
    exit
fi

kill -2 ${PID}

if [ $? -eq 0 ];then
    echo "kill $pname success"
else
    echo "kill $pname fail"
fi
