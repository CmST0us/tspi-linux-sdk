#!/bin/sh
#

TRY_CNT=0
get_pid(){
   ps -A | grep "$1" | awk '{print $1}'
}
wait_process_killed(){
   if [ "$2" = "" ]; then return; fi
   while [ "$(get_pid $1)" = "$2" ]
   do
     sleep 0.1
   done
}

check_uvc_suspend()
{
  if [ -e /tmp/uvc_goto_suspend ];then
     echo "uvc go to suspend now"
     ispserver_pid=$(get_pid ispserver)
     aiserver_pid=$(get_pid aiserver)
     killall ispserver
     killall aiserver
     wait_process_killed ispserver ${ispserver_pid}
     wait_process_killed aiserver ${aiserver_pid}
     CNT=0
     while [ "$CNT" -gt 20 ]
     do
       if [ -e /tmp/uvc_goto_suspend ];then
          sleep 0.1
          let CNT=CNT+1
       else
          CNT=100
       fi
     done
     if [ -e /tmp/uvc_goto_suspend ];then
       rm /tmp/uvc_goto_suspend -rf
       echo mem > /sys/power/state
     fi
  fi
}

check_uvc_buffer()
{
  if [ "$TRY_CNT" -gt 0 ];then
     let TRY_CNT=TRY_CNT-1
     #echo "++++++++TRY_CNT:$TRY_CNT"
  fi
  if [ "$TRY_CNT" -gt 10 ];then
     echo "+++check_uvc_buffer recovery fail,reboot to recovery now+++"
     reboot &
  fi
  if [ -e /tmp/uvc_camera_no_buf ];then
     let TRY_CNT=TRY_CNT+10
     echo "uvc no buf to send 200 frames,try to recovery isp time,timeout:$TRY_CNT"
     killall ispserver
     killall aiserver
     rm /tmp/uvc_camera_no_buf -rf
  fi
}
check_alive()
{
  if [[ ! -f "/oem/usr/bin/$1"  && ! -f "/usr/bin/$1" ]]; then
   return 1
  fi
  PID=`busybox ps |grep $1 |grep -v grep | wc -l`
  if [ $PID -le 0 ];then
     if [ "$1"x == "uvc_app"x ];then
       echo " uvc app die ,restart it and usb reprobe !!!"
       killall adbd
       killall uac_app &
       sleep 1
       killall -9 adbd
       killall -9 uac_app
       rm -rf /sys/kernel/config/usb_gadget/rockchip/configs/b.1/f*
       echo none > /sys/kernel/config/usb_gadget/rockchip/UDC
       rmdir /sys/kernel/config/usb_gadget/rockchip/functions/rndis.gs0
       rmdir /sys/kernel/config/usb_gadget/rockchip/functions/ffs.adb
       rmdir /sys/kernel/config/usb_gadget/rockchip/functions/uac*
       UDC=`ls /sys/class/udc/| awk '{print $1}'`
       echo $UDC  > /sys/bus/platform/drivers/dwc3/unbind
       echo $UDC  > /sys/bus/platform/drivers/dwc3/bind
       /oem/usb_config.sh rndis off #disable adb
       usb_irq_set
       uvc_app &
     else
       if [ "$1"x == "ispserver"x ];then
          ispserver -n &
       else
         if [ "$1"x == "aiserver"x ];then
            echo "aiserver is die,tell uvc to recovery"
            killall -3 uvc_app
            aiserver &
            sleep .5
            killall -10 smart_display_service
         else
            $1 &
         fi
       fi
     fi
  fi
}

stop_unused_daemon()
{
  killall -9 adbd
  killall -9 ntpd
  killall -9 connmand
  killall -9 dropbear
  killall -9 start_rknn.sh
  killall -9 rknn_server
}

usb_irq_set()
{
  #for usb uvc iso
  usbirq=`cat /proc/interrupts |grep dwc3| awk '{print $1}'|tr -cd "[0-9]"`
  echo "usb irq:$usbirq"
  echo 1 > /proc/irq/$usbirq/smp_affinity_list
}
#ulimit -c unlimited
dbserver &
ispserver -n &
stop_unused_daemon
#uac_app &
/oem/usb_config.sh rndis
usb_irq_set
uvc_app &
aiserver &
sleep .5
smart_display_service &
while true
do
  check_alive dbserver
  check_alive ispserver
  check_alive uvc_app
#  check_alive uac_app
  check_alive aiserver
#  check_uvc_buffer
#  check_uvc_suspend
  sleep 2
  check_alive smart_display_service
done
