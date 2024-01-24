#!/bin/bash

URI=/oem/SampleVideo_1280x720_5mb.mp4

export mpp_syslog_perror=1

if [ "$1" != "" ]
then
    URI=$1
    if [ "${URI:0:1}" != "/" ]
    then
        URI=$(readlink -f $URI)
    fi
fi

if [ "${URI:0:1}" == "/" ]
then
    URI=file://$URI
fi

while [ true ]
do
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps0 video-sink="waylandsink render-rectangle=\"<0,180,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps1 video-sink="waylandsink render-rectangle=\"<360,180,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps2 video-sink="waylandsink render-rectangle=\"<720,180,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps3 video-sink="waylandsink render-rectangle=\"<0,420,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps4 video-sink="waylandsink render-rectangle=\"<360,420,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps5 video-sink="waylandsink render-rectangle=\"<720,420,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps6 video-sink="waylandsink render-rectangle=\"<0,660,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps7 video-sink="waylandsink render-rectangle=\"<360,660,360,240>\"" text-overlay=false &
    GST_DEBUG=fps*:5 gst-launch-1.0 uridecodebin uri=$URI ! fpsdisplaysink name=fps8 video-sink="waylandsink render-rectangle=\"<720,660,360,240>\"" text-overlay=false &
    wait
done

