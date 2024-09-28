#!/bin/sh
#

check_linker()
{
	[ ! -L "$2" ] && ln -sf $1 $2
}

[ -f /etc/profile.d/enable_coredump.sh ] && source /etc/profile.d/enable_coredump.sh

check_linker /userdata   /oem/usr/www/userdata
check_linker /userdata   /oem/usr/www/userdata
check_linker /media/usb0 /oem/usr/www/usb0
check_linker /mnt/sdcard /oem/usr/www/sdcard

if [ ! -f "/oem/sysconfig.db" ]; then
  media-ctl -p -d /dev/media1 | grep 3840x2160
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-4K.db /oem/sysconfig.db
  fi
  media-ctl -p -d /dev/media1 | grep 2688x1520
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-2K.db /oem/sysconfig.db
  fi
  media-ctl -p -d /dev/media1 | grep 1920x1080
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-1080P.db /oem/sysconfig.db
  fi
  media-ctl -p -d /dev/media1 | grep 2592x1944
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-5M.db /oem/sysconfig.db
  fi
fi

#set max socket buffer size to 1.5MByte
sysctl -w net.core.wmem_max=1572864

export HDR_MODE=1
export enable_encoder_debug=0

#vpu 600M, kernel default 600M
#echo 600000000 >/sys/kernel/debug/mpp_service/rkvenc/clk_core

ipc-daemon --no-mediaserver &
sleep 2
QUICKDISPLAY=`busybox ps |grep -w startup_app_ipc |grep -v grep`
if [ -z "$QUICKDISPLAY" ] ;then
  echo "run ispserver"
  ispserver &
  sleep 1
else
  echo "ispserver is running"
fi

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

# TODO:
# HasAudio=0

if [ $HasDisplay -eq 1 ]; then
	if [ $HasHDMI -eq 1 ]; then
		mediaserver -c /oem/usr/share/mediaserver/rv1109/ipc-hdmi-display.conf &
	else
		if [ -z "$QUICKDISPLAY" ]; then
			if [ $HasAudio -eq 1 ]; then
				mediaserver -c /oem/usr/share/mediaserver/rv1109/ipc-display.conf &
			else
				mediaserver -c /oem/usr/share/mediaserver/rv1109/ipc-display-without-audio.conf &
			fi
		else
			if [ $HasAudio -eq 1 ]; then
				mediaserver -c /oem/usr/share/mediaserver/rv1109/ipc.conf &
			else
				mediaserver -c /oem/usr/share/mediaserver/rv1109/ipc-without-audio.conf &
			fi
		fi
	fi
else
	if [ $HasAudio -eq 1 ]; then
		mediaserver -c /oem/usr/share/mediaserver/rv1109/ipc.conf &
	else
		mediaserver -c /oem/usr/share/mediaserver/rv1109/ipc-without-audio.conf &
	fi
fi

# mount media part for video recording
export MEDIA_DEV=/dev/block/by-name/media
export FSTYPE=ext4

if [ ! -L $MEDIA_DEV ]; then
	echo "media part not exit, do nothing";
	exit
fi

prepare_part()
{
  dumpe2fs -h $MEDIA_DEV 2>/dev/null| grep "media"
  if [ $? -ne 0 ]; then
    echo "Auto formatting $MEDIA_DEV to $FSTYPE"
    mke2fs -F -L media $MEDIA_DEV && resize2fs $MEDIA_DEV && tune2fs -c 0 -i 0 $MEDIA_DEV && prepare_part && return
  fi
}
prepare_part
echo "prepare_part /userdata/media"
mkdir -p /userdata/media && sync
echo "fsck /userdata/media"
fsck.$FSTYPE -y $MEDIA_DEV
mount $MEDIA_DEV /userdata/media
