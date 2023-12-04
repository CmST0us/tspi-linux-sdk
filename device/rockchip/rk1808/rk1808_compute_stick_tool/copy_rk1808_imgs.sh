#!/bin/bash

SCRIPT_DIR=$(dirname $(realpath $BASH_SOURCE))
TOP_DIR=$(realpath $SCRIPT_DIR/../../../..)
BOARD_CONFIG=$TOP_DIR/device/rockchip/.BoardConfig.mk
cd $TOP_DIR
source $BOARD_CONFIG

OUT_DIR=$TOP_DIR/device/rockchip/rk1808/rk1808_compute_stick_tool
LOADER=$TOP_DIR/u-boot/*_loader_v*.bin
UBOOT=$TOP_DIR/u-boot/uboot.img
TRUST=$TOP_DIR/u-boot/trust.img
RAMBOOT_IMG=$TOP_DIR/buildroot/output/$RK_CFG_RAMBOOT/images/ramboot.img

if [ -f $LOADER ]
then
        echo -n "create loader..."
        cp $LOADER $OUT_DIR/MiniLoaderAll.bin
        echo "done."
else
        echo -e "\e[31m error: $LOADER not found,or there are multiple loaders! \e[0m"
fi

if [ -f $UBOOT ]
then
        echo -n "create uboot..."
        cp $UBOOT $OUT_DIR/uboot.img
        echo "done."
else
        echo  "warning: $UBOOT not found!"
fi

if [ -f $TRUST ]
then
        echo -n "create trust..."
        cp $TRUST $OUT_DIR/trust.img
        echo "done."
else
        echo  "warning: $TRUST not found!"
fi

if [ -f $RAMBOOT_IMG ]
then
        echo -n "create boot..."
        cp $RAMBOOT_IMG $OUT_DIR/boot.img
        echo "done."
else
        echo  "warning: $RAMBOOT_IMG not found!"
fi
