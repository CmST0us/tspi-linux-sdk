#!/bin/sh

v=$1

echo "Set HPOUT volume, range 0->30"

if [ ! -n "$v" ] ; then
	echo "please enter a volume"
else
	echo "set volume: $v"
	tinymix set "DAC HPOUT Left Volume" $v
	tinymix set "DAC HPOUT Right Volume" $v
	tinymix get "DAC HPOUT Left Volume"
	tinymix get "DAC HPOUT Right Volume"
fi
