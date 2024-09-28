#!/bin/bash

OLD_IMG=$1
NEW_IMG=$2
DIFF_IMG=$3

if [ x$OLD_IMG = x"" -o x$NEW_IMG = x"" -o x$DIFF_IMG = x"" ]; then
  echo "Usage: $0 [old_img] [new_img] [diff_img]"
  exit 1
fi

echo "To differ image from OLD: $1 and NEW: $2"

echo_red()
{
  echo -e "\033[31m$1\033[0m"
}

DIR=$(dirname $0)
export PATH=$PATH:"$DIR"

check_tool()
{
  which "$1" &> /dev/null
  if [ $? -ne 0 ]; then
    echo "command: $1 not found!"
    exit 1;
  fi
}

get_fs_type()
{
  file_info=$(file -b $1)
  if [[ "$file_info" == *"Squashfs"* ]]; then
    echo -n Squashfs
  elif [[ "$file_info" == *"ext4"* ]]; then
    echo -n ext4
  else
    echo -n unsupported
  fi
}

check_tool bsdiff
check_tool rkImageMaker
check_tool afptool
check_tool md5sum

OLD_DIR=$(mktemp -d --suffix=-OLD -p ${DIR})
NEW_DIR=$(mktemp -d --suffix=-NEW -p ${DIR})
DIFF_DIR=$(mktemp -d --suffix=-DIFF -p ${DIR})

cleanup()
{
  echo -e "\033[41;33m $1 \033[0m"
  exit 1
}

#
get_chip_from_parameter()
{
  parameter=$1

  chip=$(eval echo $(grep -h "MACHINE_MODEL" $parameter | cut -f2 -d':' | tr 'a-z' 'A-Z'))
  case $chip in
    PX30 | RK3358)
      echo -n -RKPX30
      ;;
    RK1808 | RK3399PRO_NPU)
      echo -n -RK180A
      ;;
    RK3036)
      echo -n -RK303A
      ;;
    RK3126 | RK3128)
      echo -n -RK312A
      ;;
    RK3128H)
      echo -n -RK312X
      ;;
    RK3229)
      echo -n -RK3229
      ;;
    RK3288)
      echo -n -RK320A
      ;;
    RK3308)
      echo -n -RK3308
      ;;
    RK3326)
      echo -n -RK3326
      ;;
    RK3328)
      echo -n -RK322H
      ;;
    RK3399)
      echo -n -RK330C
      ;;
    RK3568)
      echo -n -RK3568
      ;;
    RK3588)
      echo -n -RK3588
      ;;
    RV1126)
      echo -n -RK1126
      ;;
    *)
      cleanup "Bad MACHINE_MODEL: $chip in parameter.txt"
  esac
}

# diff_img <name> <old_file> <new_file> <diff_file>
diff_img()
{
  name=$1
  diff=$4
  old=$2
  new=$3
  old_size=$(stat --printf=%s $old)
  new_size=$(stat --printf=%s $new)
  md5sum=$(md5sum $new | cut -f 1 -d ' ')

  echo_red "Diff ${NAME}..."
  sec_1=$(date +%s)
  bsdiff "$old" "$new" "${diff}" || cleanup "bsdiff $new: $?"
  sec_2=$(date +%s)
  # MAGIC HEADER 80 bytes
  printf "DIFF:%-15s:%-12s:%-12s:%-32s:" $name $old_size $new_size $md5sum  >> "${diff}"
  echo_red "Diff ${NAME} use $((($sec_2 - $sec_1)/3600))h:$((($sec_2 - $sec_1)%3600/60))m:$((($sec_2 - $sec_1)%60))s"
}

echo "Unpacking old image: ${OLD_IMG}"
rkImageMaker -unpack ${OLD_IMG} ${OLD_DIR} || cleanup "rkImageMaker unpack: $?"
afptool -unpack ${OLD_DIR}/firmware.img ${OLD_DIR} || cleanup "afptool unpack: $?"

echo "Unpacking new image: ${NEW_IMG}"
rkImageMaker -unpack ${NEW_IMG} ${NEW_DIR} || cleanup "rkImageMaker unpack: $?"
afptool -unpack ${NEW_DIR}/firmware.img ${NEW_DIR} || cleanup "afptool unpack: $?"

# TODO: a/b image is not yet supported, warning

mkdir ${DIFF_DIR}/Image

# Copy parameter/loader to target dir
cp ${NEW_DIR}/package-file ${DIFF_DIR}/
cp ${NEW_DIR}/MiniLoaderAll.bin ${DIFF_DIR}/
cp ${NEW_DIR}/MiniLoaderAll.bin ${DIFF_DIR}/Image
cp ${NEW_DIR}/parameter.txt ${DIFF_DIR}/
cp ${NEW_DIR}/parameter.txt ${DIFF_DIR}/Image

while read LINE; do
  # delete any blank at beginning or "#" line
  STRIP=$(eval echo "$LINE")
  if [[ $STRIP = "" ]]; then
    continue
  fi

  # get the paratition name by deleting the chars after [[:blank:]]
  NAME=${STRIP/%[[:blank:]]*/}
  IMG=${STRIP/#*[[:blank:]]/}

  if [[ $NAME = "backup" ]]; then
    echo_red "Skip backup, it's not a real image"
    continue
  fi

  if [ $NAME = "package-file" -o $NAME = "parameter" -o $NAME = "bootloader" ]; then
    echo_red "Copy ${IMG}"
    # Already copied
    continue
  fi

  if [ ! -f ${OLD_DIR}/${IMG} ]; then
    echo_red "Copy ${IMG}, there's not corresponding old file"
    ln -s ../../${NEW_DIR}/${IMG} ${DIFF_DIR}/${IMG}
    continue
  fi

  case ${NAME} in
    uboot | trust | boot)
      diff_img $NAME ${OLD_DIR}/${IMG} ${NEW_DIR}/${IMG} ${DIFF_DIR}/${IMG}
      ;;
    rootfs)
      fs_new=$(get_fs_type ${NEW_DIR}/${IMG})
      fs_old=$(get_fs_type ${OLD_DIR}/${IMG})

      if [ $fs_new != $fs_old ]; then
        echo_red "Copy ${IMG}, root fs type are different: $fs_old vs $fs_new"
        ln -s ../../${NEW_DIR}/${IMG} ${DIFF_DIR}/${IMG}
      elif [ $fs_new != "Squashfs" ]; then
        echo_red "Copy ${IMG}, $fs_new not supported"
        ln -s ../../${NEW_DIR}/${IMG} ${DIFF_DIR}/${IMG}
      else
        diff_img $NAME ${OLD_DIR}/${IMG} ${NEW_DIR}/${IMG} ${DIFF_DIR}/${IMG}
      fi
      ;;
    *)
      echo_red "Copy ${IMG}"
      ln -s ../../${NEW_DIR}/${IMG} ${DIFF_DIR}/${IMG}
      ;;
  esac
done < ${NEW_DIR}/package-file

afptool -pack ${DIFF_DIR} ${DIFF_DIR}/Image/update.img || cleanup "afptool pack: $?"
rkImageMaker $(get_chip_from_parameter ${DIFF_DIR}/parameter.txt) \
  ${DIFF_DIR}/Image/MiniLoaderAll.bin \
  ${DIFF_DIR}/Image/update.img \
  ${DIFF_IMG} -os_type:androidos || cleanup "rkImageMaker pack: $?"

#rm -rf ${DIFF_DIR} ${NEW_DIR} ${OLD_DIR}

echo_red "Make diff ${DIFF_IMG} Finished"
