#!/bin/sh

v=$1

echo "Set HPMIX volume, range 0->1"

if [ ! -n "$v" ] ; then
	echo "please enter a volume"
else
	echo "set volume: $v"
	tinymix set "DAC HPMIX Left Volume" $v
	tinymix set "DAC HPMIX Right Volume" $v
	tinymix get "DAC HPMIX Left Volume"
	tinymix get "DAC HPMIX Right Volume"
fi
