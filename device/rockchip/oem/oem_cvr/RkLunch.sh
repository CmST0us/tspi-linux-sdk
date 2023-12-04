#!/bin/sh
#

[ -f /etc/profile.d/enable_coredump.sh ] && source /etc/profile.d/enable_coredump.sh

export enable_encoder_debug=0

export rt_vo_disable_vop=0

amixer -c 0 sset "Playback Path" SPK

amixer -c 0 sset "Capture MIC Path" "Main Mic"

LD_PRELOAD=/oem/libthird_media.so cvr_app &
