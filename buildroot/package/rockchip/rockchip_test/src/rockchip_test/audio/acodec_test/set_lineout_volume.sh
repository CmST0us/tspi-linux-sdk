#!/bin/sh

v=$1

echo "Set LINEOUT volume, range 0->3"

if [ ! -n "$v" ] ; then
	echo "please enter a volume"
else
	echo "set volume: $v"
	tinymix set "DAC LINEOUT Left Volume" $v
	tinymix set "DAC LINEOUT Right Volume" $v
	tinymix get "DAC LINEOUT Left Volume"
	tinymix get "DAC LINEOUT Right Volume"
fi
