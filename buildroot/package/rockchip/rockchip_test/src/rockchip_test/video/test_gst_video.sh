#!/bin/sh
gst-launch-1.0 filesrc location=/oem/SampleVideo_1280x720_5mb.mp4 ! qtdemux ! h264parse ! mppvideodec ! waylandsink
#gst-launch-1.0 playbin uri=file:///oem/SampleVideo_1280x720_5mb.mp4 audio-sink=fakesink
