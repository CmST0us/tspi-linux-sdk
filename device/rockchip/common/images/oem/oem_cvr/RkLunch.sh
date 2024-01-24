#!/bin/sh
#

[ -f /etc/profile.d/enable_coredump.sh ] && source /etc/profile.d/enable_coredump.sh

export enable_encoder_debug=0

export rt_vo_disable_vop=0

media-ctl -p -d /dev/media1 | grep 3840x2160
if [ $? -eq 0 ] ;then
    ln -s -f /oem/etc/rkadk/8M/rkadk_setting_sensor_0.ini /oem/etc/rkadk/rkadk_setting_sensor_0.ini
fi
media-ctl -p -d /dev/media1 | grep 2592x1944
if [ $? -eq 0 ] ;then
    ln -s -f /oem/etc/rkadk/5M/rkadk_setting_sensor_0.ini /oem/etc/rkadk/rkadk_setting_sensor_0.ini
fi

LD_PRELOAD=/oem/libthird_media.so cvr_app &
