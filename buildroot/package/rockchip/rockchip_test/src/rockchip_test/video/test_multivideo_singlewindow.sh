#!/bin/sh


case "$1" in
	test)
		multivideoplayer /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4 /oem/SampleVideo_1280x720_5mb.mp4
		;;
	$1)
		multivideoplayer $1 $1 $1 $1 $1 $1 $1 $1 $1
		;;
esac
shift
