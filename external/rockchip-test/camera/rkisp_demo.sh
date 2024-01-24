#!/bin/bash

pkill rkaiq_3A_server

MEDIA_NUM=$(ls /dev/media* | wc -l)

if [[ $MEDIA_NUM == 2 ]];then
	rkisp_demo --device /dev/video11 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
elif [[ $MEDIA_NUM == 4 ]];then
	rkisp_demo --device /dev/video22 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
	rkisp_demo --device /dev/video31 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
elif [[ $MEDIA_NUM == 6 ]];then
    	rkisp_demo --device /dev/video33 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
    	rkisp_demo --device /dev/video42 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
    	rkisp_demo --device /dev/video51 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
elif [[ $MEDIA_NUM == 8 ]];then
    	rkisp_demo --device /dev/video44 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
    	rkisp_demo --device /dev/video53 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
    	rkisp_demo --device /dev/video62 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
    	rkisp_demo --device /dev/video71 --width 1920 --height 1080 --rkaiq --iqpath=/etc/iqfiles/ 2>&1 &
fi
