#!/bin/bash

COMMON_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        COMMON_DIR=$(dirname $CMD)
fi
cd $COMMON_DIR
cd ../../..
TOP_DIR=$(pwd)
echo "config is $RK_CFG_BUILDROOT"
if [ -z $RK_CFG_BUILDROOT ]
then
	echo "config for building doesn't exist, skip!"
	exit 0
fi

source $TOP_DIR/buildroot/build/envsetup.sh $RK_CFG_BUILDROOT
make

KERNEL_IMAGE=$TOP_DIR/$RK_KERNEL_IMG
KERNEL_DTB=$TOP_DIR/kernel/resource.img
UBOOT_DIR=$TOP_DIR/u-boot/
CPIO_IMG=$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/images/rootfs.cpio.gz
TARGET_IMAGE=$TOP_DIR/rockdev/$MULTINPU_BOOT_IMG
TARGET_DIR=$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/target/opt

echo -n "pack $MULTINPU_BOOT_IMG..."

if [ -f $TARGET_DIR/boot.img ]
then
	echo "remove old boot image"
	rm $TARGET_DIR/boot.img
fi

# build kernel
if [ -f $KERNEL_IMAGE ]
then
	echo "found kernel image"
else
	echo "kernel image doesn't exist, now build kernel image"
	$TOP_DIR/build.sh kernel
	if [ $? -eq 0 ]; then
		echo "build kernel done"
	else
		exit 1
	fi
fi

# build uboot
if [ -f $UBOOT_DIR/uboot.img ]
then
	echo "found uboot image"
else
	echo "uboot image doesn't exist, now build uboot image"
	$TOP_DIR/build.sh uboot
	if [ $? -eq 0 ]; then
		echo "build uboot done"
	else
		exit 1
	fi
fi

cp $UBOOT_DIR/uboot.img $TARGET_DIR/uboot.img
cp $UBOOT_DIR/trust.img $TARGET_DIR/trust.img
cp $UBOOT_DIR/rk3399pro_npu_loader* $TARGET_DIR/MiniLoaderAll.bin
cp $KERNEL_IMAGE $TARGET_DIR/

# check if follower board need other dts
if [ -z $RK_KERNEL_SECOND_DTS ]
then
	cp $KERNEL_DTB $TARGET_DIR/
else
	cd $TOP_DIR/kernel
	make
	scripts/resource_tool --image=second_resource.img arch/arm64/boot/dts/rockchip/$RK_KERNEL_SECOND_DTS.dtb
	cp second_resource.img $TARGET_DIR/resource.img
	cd $TOP_DIR
fi

make
$TOP_DIR/kernel/scripts/mkbootimg --kernel $KERNEL_IMAGE --ramdisk $CPIO_IMG --second $KERNEL_DTB -o $TARGET_DIR/boot.img
echo "done."
