#!/bin/sh

echo "Loop playback $1"

while [ true ]
do
	tinyplay $1 -r 44100 &
	sleep 2
	/data/stop_tinyplay.sh
	sleep 2
done;
