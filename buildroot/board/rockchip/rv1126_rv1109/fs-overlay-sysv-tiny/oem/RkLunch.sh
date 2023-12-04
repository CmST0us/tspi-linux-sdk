#!/bin/sh
#

#set max socket buffer size to 1.5MByte
sysctl -w net.core.wmem_max=1572864

export HDR_MODE=1
export enable_encoder_debug=0

killall udhcpc
udhcpc -b -i eth0 

ispserver -no-sync-db &
sleep 1

ls /sys/class/drm | grep "card0-"
if [ $? -ne 0 ] ;then
  echo "not found display"
  HasDisplay=0
else
  echo "find display"
  HasDisplay=1
  cat /proc/device-tree/compatible | grep lt9611
  if [ $? -ne 0 ] ;then
    echo "not HDMI"
  else
    echo "find HDMI"
    HasHDMI=1
  fi
fi

arecord -l |grep "card 0"
if [ $? -ne 0 ] ;then
  echo "not found sound card"
  HasAudio=0
else
  echo "find sound card"
  HasAudio=1
fi

# default disable audio
HasAudio=0

dst_w=2688
dst_h=1520
media-ctl -p -d /dev/media1 | grep 3840x2160
if [ $? -eq 0 ] ;then
	dst_w=3840
	dst_h=2160
fi
media-ctl -p -d /dev/media1 | grep 2688x1520
if [ $? -eq 0 ] ;then
	dst_w=2688
	dst_h=1520
fi
media-ctl -p -d /dev/media1 | grep 1920x1080
if [ $? -eq 0 ] ;then
	dst_w=1920
	dst_h=1080
fi

dst_conf=ipc-without-audio-tiny.conf
if [ $HasDisplay -eq 1 ]; then
	if [ $HasHDMI -eq 1 ]; then
		dst_conf=ipc-hdmi-display-tiny.conf
	else
		dst_conf=ipc-display-tiny.conf
	fi
else
	if [ $HasAudio -eq 1 ]; then
		dst_conf=ipc-tiny.conf
	else
		dst_conf=ipc-without-audio-tiny.conf
	fi
fi

ln -fs /oem/usr/share/mediaserver/rv1109/$dst_conf /tmp/$dst_conf
cmd="sed -i '/2688/s/2688/$dst_w/g' /tmp/$dst_conf"
eval $cmd
cmd="sed -i '/1520/s/1520/$dst_h/g' /tmp/$dst_conf"
eval $cmd
mediaserver -a -d -c /tmp/$dst_conf &
