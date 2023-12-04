#!/bin/sh

adc=$1
v=$2
ch=

echo "Set ADC MIC volume, ADC: 0~7, volume range 0->3"

case $adc in
	0 | 2 | 4 | 6)
		ch="Left"
		;;
	1 | 3 | 5 | 7)
		ch="Right"
		;;
esac

if [ $adc -gt 7 ]; then
	echo "ERR!! The ADC range is 0~7"
	exit 1;
elif [ $adc -eq 0 ]; then
	echo "adc is 0"
	grp=0;
else
	echo "adc $adc > 0"
	grp=$(($adc/2));
fi

echo "Will set $ch adc: $adc"

if [ ! -n "$v" ] ; then
	echo "ERR: please enter a volume"
else
	content="ADC MIC Group $grp $ch Volume"
	echo "Set ADC $adc, content: $content: $v"
	tinymix set "$content" $v
	tinymix get "$content"
fi
