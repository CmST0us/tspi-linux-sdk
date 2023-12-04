#!/bin/bash

set -e

rk_ramdisk_security_bootup_normal()
{
	echo "[$0] Build ramdisk with sha256 digest"
	ROOTFS_IMAGE_DIGEST=$(dirname $ROOTFS_IMAGE)/ramdisk.gz.digest

	openssl dgst -sha256 -binary -out $ROOTFS_IMAGE_DIGEST $ROOTFS_IMAGE || exit 1
	rootfs_image_digest_size=$(du -b $ROOTFS_IMAGE |sed -r -e 's/[[:space:]]+.*$//')

	if [ "$RK_ARCH" == "arm" ]; then
		kernel_dts_file="kernel/arch/arm/boot/dts/$RK_KERNEL_DTS.dts"
	else
		kernel_dts_file="kernel/arch/arm64/boot/dts/rockchip/$RK_KERNEL_DTS.dts"
	fi

	cp $kernel_dts_file ${kernel_dts_file}.backup
cat << EOF >> ${kernel_dts_file}
&ramdisk_c {
	size = <$rootfs_image_digest_size>;
	hash {
		algo = "sha256";
		value = /incbin/("$ROOTFS_IMAGE_DIGEST");
	};
};
EOF
	./build.sh kernel
	mv ${kernel_dts_file}.backup $kernel_dts_file
}

rk_ramdisk_build_init()
{
	echo "Try to build init for $1"

	if [ "$RK_SYSTEM_CHECK_METHOD" = "DM-V" ]; then
		SYSTEM_IMAGE=$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/images/rootfs.squashfs
	else
		SYSTEM_IMAGE=$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/images/rootfs.${RK_ROOTFS_TYPE}
	fi
	if [ ! -e "$SYSTEM_IMAGE" ]; then
		echo "ERROR: Please build system first"
		exit -1
	fi

	$COMMON_DIR/mk-dm.sh $1 $SYSTEM_IMAGE
}

COMMON_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        COMMON_DIR=$(dirname $CMD)
fi
cd $COMMON_DIR
cd ../../..
TOP_DIR=$(pwd)
RAMDISK_IMG=$1
RAMDISK_CFG=$2
RAMDISK_TYPE=$RK_RAMBOOT_TYPE
echo "config is $RAMDISK_CFG"
if [ -z $RAMDISK_CFG ]
then
	echo "config for building $RAMDISK_IMG doesn't exist, skip!"
	exit 0
fi

BOARD_CONFIG=$TOP_DIR/device/rockchip/.BoardConfig.mk
source $BOARD_CONFIG
if [ -z $RK_KERNEL_ZIMG ]; then
	KERNEL_IMAGE=$TOP_DIR/$RK_KERNEL_IMG
else
	KERNEL_IMAGE=$TOP_DIR/$RK_KERNEL_ZIMG
fi

KERNEL_DTB=$TOP_DIR/kernel/resource.img

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

source $TOP_DIR/buildroot/build/envsetup.sh $RAMDISK_CFG
CPIO_IMG=$TOP_DIR/buildroot/output/$RAMDISK_CFG/images/rootfs.cpio.gz
ROMFS_IMG=$TOP_DIR/buildroot/output/$RAMDISK_CFG/images/rootfs.romfs
TARGET_IMAGE=$TOP_DIR/buildroot/output/$RAMDISK_CFG/images/$RAMDISK_IMG

if [ -z $RAMDISK_TYPE ]
then
RAMDISK_TYPE=CPIO
fi

eval ROOTFS_IMAGE=\$${RAMDISK_TYPE}_IMG

if [ "$RK_RAMDISK_SECURITY_BOOTUP" = "true" ];then
	case "$RK_SYSTEM_CHECK_METHOD" in
	"DM-V" | "DM-E")
		rk_ramdisk_build_init $RK_SYSTEM_CHECK_METHOD
		;;
	*)
		echo "do nothing ($RK_SYSTEM_CHECK_METHOD)"
	esac
fi

# build ramdisk
echo "====Start build $RAMDISK_CFG===="
$TOP_DIR/buildroot/utils/brmake
if [ $? -eq 0 ]; then
    echo "log saved on $TOP_DIR/br.log"
    echo "====Build $RAMDISK_CFG ok!===="
else
    echo "log saved on $TOP_DIR/br.log"
    echo "====Build $RAMDISK_CFG failed!===="
    tail -n 100 $TOP_DIR/br.log
    exit 1
fi

if [ $RAMDISK_TYPE == ROMFS ]
then
# Do compress for tinyrootfs
cat $ROOTFS_IMAGE | gzip -n -f -9 > $ROOTFS_IMAGE.gz
cat $KERNEL_IMAGE | gzip -n -f -9 > $KERNEL_IMAGE.gz
ROOTFS_IMAGE=$ROOTFS_IMAGE.gz
KERNEL_IMAGE=$KERNEL_IMAGE.gz
fi

echo -n "pack $RAMDISK_IMG..."
if [ -f "$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/$RK_RECOVERY_FIT_ITS" ];then
	if [ "$RK_RAMDISK_SECURITY_BOOTUP" = "true" ];then
		if [ -z "$RK_SYSTEM_CHECK_METHOD" ]; then
			rk_ramdisk_security_bootup_normal $0
		fi
	fi

	$COMMON_DIR/mk-fitimage.sh $TARGET_IMAGE $TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/$RK_RECOVERY_FIT_ITS $ROOTFS_IMAGE $KERNEL_IMAGE
else
	$TOP_DIR/kernel/scripts/mkbootimg --kernel $KERNEL_IMAGE --ramdisk $ROOTFS_IMAGE --second $KERNEL_DTB -o $TARGET_IMAGE
fi
echo "done."
