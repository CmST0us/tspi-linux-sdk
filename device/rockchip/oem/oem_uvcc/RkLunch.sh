#!/bin/sh
#

#vi_aging adjust
#io -4 0xfe801048 0x40

#export VIV_VX_ENABLE_NN_DDR_BURST_SIZE_256B=0
#export VIV_VX_MAX_SOC_OT_NUMBER=16

if [ -e /sys/firmware/devicetree/base/__symbols__/gc4c33 ] ;then
  echo "isp sensor is gc4c33,disable HDR"
  export HDR_MODE=0
else
if [ -e /sys/firmware/devicetree/base/__symbols__/ov5695 ] ;then
  echo "isp sensor is ov5695,disable HDR"
  export HDR_MODE=0
else
if [ -e /sys/firmware/devicetree/base/__symbols__/os04a10 ] ;then
  echo "isp sensor is os04a10,enable HDR"
  export HDR_MODE=1
else
if [ -e /sys/firmware/devicetree/base/__symbols__/imx347 ] ;then
  echo "isp sensor is imx347,enable HDR"
  export HDR_MODE=1
else
if [ -e /sys/firmware/devicetree/base/__symbols__/ov4689 ] ;then
  echo "isp sensor is ov4689,enable HDR"
  export HDR_MODE=1
else
  echo "unkonw sensor,disable HDR default"
  export HDR_MODE=0
fi
fi
fi
fi
fi

#init sysconfig.db
if [ ! -e "/data/sysconfig.db" ] ;then
   cp -rf /oem/sysconfig.db /data/sysconfig.db
fi

camera_max_width=`media-ctl -p | grep crop|head -1|awk -F '[/@x]' '{print $2}'`
camera_max_height=`media-ctl -p | grep crop|head -1|awk -F '[/@x]' '{print $3}'`

echo "camera_max_width= ${camera_max_width}"
echo "camera_max_height= ${camera_max_height}"
export CAMERA_MAX_WIDTH=${camera_max_width}
export CAMERA_MAX_HEIGHT=${camera_max_height}

#line config
isp_line=0 #$((${camera_max_height} / 2))
ispp_line=0  #$((${camera_max_height} / 4 * 3 ))    #no fec
cif_line=$((${camera_max_height} / 4 * 3 ))
echo "isp_line= $isp_line , ispp_line= $isp_line, cif_line= $cif_line"
echo $isp_line > /sys/module/video_rkisp/parameters/wait_line
echo $ispp_line > /sys/module/video_rkispp/parameters/wait_line
echo $cif_line > /sys/devices/platform/rkcif_mipi_lvds/wait_line

#rockit log level ctrl: 1:fatal error; 2: error; 3: warning; 4:infomational; 5:debug level; 6:verbose
export rt_log_level=3
#export uvc_app_log_level=3

#export ENABLE_EPTZ=1

/oem/aicamera.sh &
