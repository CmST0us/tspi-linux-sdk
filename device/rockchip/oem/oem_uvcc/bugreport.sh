#!/bin/bash
#

bugreport=/userdata/bugreport

usage()
{
    echo "USAGE: [-s] [-u] [-i] [-a] [-m] [-p] [-h]              "
    echo "No ARGS means bugreport all infos in '${bugreport}'    "
    echo "WHERE: -s = report system info                         "
    echo "       -u = report usb info                            "
    echo "       -i = report isp info                            "
    echo "       -a = report app info                            "
    echo "       -m = report mpp enc use time and in&out data    "
    echo "       -p = path to save bugreport infos               "
    echo "       -h = bugreport using help                       "
    echo "EXAMPLE: bugreport.sh -p /tmp/bugreport -suia          "
    exit 1
}

get_uvc_enc_in()
{
  touch /tmp/uvc_enc_in
  sleep 1
  rm /tmp/uvc_enc_in
  if [ ! -d ${bugreport}/uvc_data ];then
    mkdir -p ${bugreport}/uvc_data
  fi
  mv /userdata/uvc_enc_in.bin ${bugreport}/uvc_data/
  sync
}

get_uvc_enc_out()
{
  touch /tmp/uvc_enc_out
  sleep 3
  rm /tmp/uvc_enc_out
  if [ ! -d ${bugreport}/uvc_data ];then
    mkdir -p ${bugreport}/uvc_data
  fi
  mv /userdata/uvc_enc_out.bin ${bugreport}/uvc_data/
  sync
}

get_mpp_status()
{
  if [ ! -d ${bugreport}/uvc_data ];then
    mkdir -p ${bugreport}/uvc_data
  fi
  echo 0x100 > /sys/module/rk_vcodec/parameters/mpp_dev_debug
  sleep 5
  echo 0 > /sys/module/rk_vcodec/parameters/mpp_dev_debug
  if [ -e /tmp/messages ];then
    tail -n 500 /tmp/messages > ${bugreport}/uvc_data/mpp_enc_time_info
  fi
  sync
}

get_isp_status()
{
  if [ ! -d ${bugreport}/isp_data ];then
    mkdir -p ${bugreport}/isp_data
  fi
  echo "report rkisp and rkispp info..."
  cat /proc/rkisp* > ${bugreport}/isp_data/rkisp_data0
  sleep 1
  cat /proc/rkisp* > ${bugreport}/isp_data/rkisp_data1
  sleep 1
  cat /proc/rkisp* > ${bugreport}/isp_data/rkisp_data2
  echo "report rkcif info..."
  cat /proc/rkcif* > ${bugreport}/isp_data/rkcif_data0
  sleep 1
  cat /proc/rkcif* > ${bugreport}/isp_data/rkcif_data1
  sleep 1
  cat /proc/rkcif* > ${bugreport}/isp_data/rkcif_data2
  echo "report media-ctl info..."
  media-ctl -p > ${bugreport}/isp_data/media-ctl-p_info
  media-ctl -d /dev/media0 -p > ${bugreport}/isp_data/media-ctl-p-media0_info
  media-ctl -d /dev/media1 -p > ${bugreport}/isp_data/media-ctl-p-media1_info
  echo "report isp&ispp reg info..."
  io -4 -l 0x10000 0xffb50000 > ${bugreport}/isp_data/isp.reg
  io -4 -l 0x10000 0xffb60000 > ${bugreport}/isp_data/ispp.reg
  sync
}

get_usb_status()
{
  if [ ! -d ${bugreport}/usb_data ];then
    mkdir -p ${bugreport}/usb_data
  fi
  echo "report usb&uvc info..."
  echo "  uvc trace for setup request processing begin..."
  echo "    First , Need quit camera preview."
  echo "    Second, Restart camera preview..."
  echo "  Make sure the above two steps are completed within 15s"
  echo 4 > /sys/module/usb_f_uvc/parameters/trace
  sleep 15
  dmesg > ${bugreport}/usb_data/dmesg_info

  cat /proc/uvcinfo > ${bugreport}/usb_data/usb_data0
  sleep 1
  cat /proc/uvcinfo > ${bugreport}/usb_data/usb_data1
  sleep 1
  cat /proc/uvcinfo > ${bugreport}/usb_data/usb_data2
}

get_sys_status()
{
  if [ ! -d ${bugreport}/system_data ];then
    mkdir -p ${bugreport}/system_data
  fi
  echo "report system ps info..."
  ps -ef > ${bugreport}/system_data/ps_info
  ps -eLo pid,tid,class,rtprio,ni,pri,psr,pcpu,pmem,stat,wchan:30,comm > ${bugreport}/system_data/ps_detail_info
  echo "report system mem free info..."
  free -h > ${bugreport}/system_data/free_info
  echo "report system flash mount info..."
  mount > ${bugreport}/system_data/mount_info
  echo "report system flash using info..."
  df > ${bugreport}/system_data/df_info
  echo "report system clk_summary info..."
  cat /sys/kernel/debug/clk/clk_summary > ${bugreport}/system_data/clk_summary_info0
  sleep 1
  cat /sys/kernel/debug/clk/clk_summary > ${bugreport}/system_data/clk_summary_info1
  echo "report system dmesg info..."
  dmesg > ${bugreport}/system_data/dmesg_info
  if [ -e /tmp/messages ];then
    echo "report system syslog info..."
    cp /tmp/messages  ${bugreport}/system_data/syslog
  fi
  echo "report system top info..."
  top -b > ${bugreport}/system_data/top_info &
  sleep 2
  killall top
  sync
}

get_app_status(){
  if [ ! -d ${bugreport}/application_data ];then
    mkdir -p ${bugreport}/application_data
  fi
  echo "report applicattion $1 info..."
  pid=`ps -ef | grep $1 | grep -v "grep" | awk '{print $2}'`
  top -b -H -p ${pid} > ${bugreport}/application_data/$1_info &
  echo "totol fd nums and max fd supprot" > ${bugreport}/application_data/$1_fd_info
  cat /proc/sys/fs/file-nr >> ${bugreport}/application_data/$1_fd_info
  echo "fd nums in $1" >> ${bugreport}/application_data/$1_fd_info
  ls /proc/${pid}/fd | wc -l >> ${bugreport}/application_data/$1_fd_info
  echo "detail fds in $1" >> ${bugreport}/application_data/$1_fd_info
  ls -l /proc/${pid}/fd >> ${bugreport}/application_data/$1_fd_info
  sleep 3
  killall top
  if [ "$1"x == "uvc_app"x ];then
    echo "uvc_app version > V1.31 show RGB in host start..."
    touch /tmp/uvc_isp_state
    sleep 3
    rm /tmp/uvc_isp_state
    echo "uvc_app version > V1.31 show RGB in host end..."
    echo "report uvc_app uvc_ipc_state info..."
    touch /tmp/uvc_ipc_state
    sleep 5
    rm /tmp/uvc_ipc_state
    if [ -e /tmp/messages ];then
      tail -n 500 /tmp/messages > ${bugreport}/application_data/$1_uvc_ipc_state_info
    fi
    echo "report uvc_app uvc_out_len info..."
    touch /tmp/uvc_out_len
    sleep 5
    rm /tmp/uvc_out_len
    if [ -e /tmp/messages ];then
      tail -n 500 /tmp/messages > ${bugreport}/application_data/$1_uvc_out_len_info
    fi
    echo "report uvc_app uvc_use_time info..."
    touch /tmp/uvc_use_time
    sleep 3
    rm /tmp/uvc_use_time
    if [ -e /tmp/messages ];then
      tail -n 500 /tmp/messages > ${bugreport}/application_data/$1_uvc_use_time_info
    fi
    sync
  fi
}

report_all_info()
{
  get_sys_status
  get_usb_status
  get_uvc_enc_in
  get_uvc_enc_out
  get_isp_status
  get_app_status aiserver
  get_app_status ispserver
  get_app_status uvc_app
  get_app_status smart_display_service
}

if [ $# -ne 0 ];then
  while getopts "suiamp:h" arg
  do
    case $arg in
      s)
        echo "report system info"
        get_sys_status
        ;;
      u)
        echo "report usb info"
        get_usb_status
        ;;
      i)
        echo "report isp info"
        get_isp_status
        ;;
      a)
        echo "report app info"
        get_app_status aiserver
        get_app_status ispserver
        get_app_status uvc_app
        get_app_status smart_display_service
        ;;
      m)
        echo "report enc use time and in&out data"
        get_uvc_enc_in
        get_uvc_enc_out
        get_mpp_status
        ;;
      p)
        echo "save bugreport infos in path '${OPTARG}'"
        bugreport=${OPTARG}
        if [ $# -eq 2 ];then
          report_all_info
        fi
        ;;
      h)
        usage
        ;;
      ?)
        usage
        ;;
      esac
  done
else
  echo "report all infos"
  report_all_info
fi

echo "done, bugreport saved in '${bugreport}'"



