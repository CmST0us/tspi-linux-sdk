#!/bin/sh

v=$1

echo "Usage:"
echo "	input <-- mi: mic-in, li: line-in"
echo "	output --> lo: line-out, hp: hp-out. lohp: both line-out and hp-out"
echo ""

NODE_PATH=/sys/devices/platform/ff560000.acodec/rk3308-acodec-dev
cd $NODE_PATH

if [ -z $v ] ; then
	echo "show status for all paths:"
	cat adc_grp0_in
	cat dac_output
elif [ $v = "mi" ] ; then
	echo "switch to mic-in"
	echo 0 > adc_grp0_in
	cat adc_grp0_in
elif [ $v = "li" ] ; then
	echo "switch to line-in"
	echo 1 > adc_grp0_in
	cat adc_grp0_in
elif [ $v = "lo" ] ; then
	echo "switch to line-out"
	echo 0 > dac_output
	cat dac_output
elif [ $v = "hp" ] ; then
	echo "switch to hp-out"
	echo 1 > dac_output
	cat dac_output
elif [ $v = "lohp" ] ; then
	echo "enable both line-out and hp-out"
	echo 11 > dac_output
	cat dac_output
else
	echo "invaild param"
fi
