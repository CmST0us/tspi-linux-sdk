#! /bin/bash


DEVICE_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        DEVICE_DIR=$(dirname $CMD)
fi
cd $DEVICE_DIR
cd ../../..
TOP_DIR=$(pwd)

export RK_LOADER_NAME=
source $TOP_DIR/device/rockchip/.BoardConfig.mk

ROCKDEV=$TOP_DIR/rockdev
PRODUCT_PATH=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT
PARAMETER=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/$RK_PARAMETER
OEM_DIR=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/rk3308/$RK_OEM_DIR
USER_DATA_DIR=$TOP_DIR/device/rockchip/userdata/$RK_USERDATA_DIR
MISC_IMG=$TOP_DIR/device/rockchip/rockimg/wipe_all-misc.img
ROOTFS_IMG=$TOP_DIR/$RK_ROOTFS_IMG
RECOVERY_IMG=$TOP_DIR/buildroot/output/$RK_CFG_RECOVERY/images/recovery.img
TRUST_IMG=$TOP_DIR/u-boot/trust.img
UBOOT_IMG=$TOP_DIR/u-boot/uboot.img
BOOT_IMG=$TOP_DIR/kernel/$RK_BOOT_IMG
if [ -z $RK_LOADER_NAME ]; then
	LOADER=$TOP_DIR/u-boot/*_loader_v*.bin
else
	LOADER=$TOP_DIR/u-boot/$RK_LOADER_NAME
fi
MKIMAGE=$TOP_DIR/device/rockchip/common/mk-image.sh
rm -rf $ROCKDEV
mkdir -p $ROCKDEV

# Require buildroot host tools to do image packing.
if [ ! -d "$TARGET_OUTPUT_DIR" ]; then
    echo "Source buildroot/build/envsetup.sh"
    source $TOP_DIR/buildroot/build/envsetup.sh $RK_CFG_BUILDROOT
fi

if [ "${RK_OEM_DIR}" == "dueros"  ];then
	if [ $RK_ARCH == arm ];then
		TARGET_ARM_TYPE=arm32
	else
		TARGET_ARM_TYPE=arm64
		PARAMETER=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/parameter-64bit-dueros.txt
	fi
	OEM_DIR=${ROCKDEV}/.oem
	rm -rf ${OEM_DIR}
	mkdir -p ${OEM_DIR}
	find ${PRODUCT_PATH}/${RK_OEM_DIR} -maxdepth 1 -not -name "spil" \
        	-not -wholename "${PRODUCT_PATH}/${RK_OEM_DIR}" \
        	-exec sh -c 'cp -arf ${0} ${1}' "{}" ${OEM_DIR} \;
	cp -rf ${PRODUCT_PATH}/${RK_OEM_DIR}/spil/${TARGET_ARM_TYPE}/baidu_spil_rk3308_${MIC_NUM}mic ${OEM_DIR}/baidu_spil_rk3308
	echo "copy ${TARGET_ARM_TYPE} with ${MIC_NUM}mic."
else
	OEM_DIR=${PRODUCT_PATH}/${RK_OEM_DIR}
fi


if [ -f $ROOTFS_IMG ]
then
	echo -n "create rootfs.img..."
	cp -aL $ROOTFS_IMG $ROCKDEV/rootfs.img
	echo "done."
else
	echo -e "\e[31m error: $ROOTFS_IMG not found! \e[0m"
fi

if [ -f $PARAMETER ]
then
	echo -n "create parameter..."
	cp -a $PARAMETER $ROCKDEV/parameter.txt
	echo "done."
else
	echo -e "\e[31m error: $PARAMETER not found! \e[0m"
fi

if [ -f $RECOVERY_IMG ]
then
	echo -n "create recovery.img..."
	cp -a $RECOVERY_IMG $ROCKDEV/recovery.img
	echo "done."
else
	echo -e "\e[31m error: $RECOVERY_IMG not found! \e[0m"
fi

if [ -f $MISC_IMG ]
then
	echo -n "create misc.img..."
	cp -a $MISC_IMG $ROCKDEV/misc.img
	echo "done."
else
	echo -e "\e[31m error: $MISC_IMG not found! \e[0m"
fi
echo -n "$OEM_DIR $RK_OEM_FS_TYPE"
if [ -d $OEM_DIR ]
then
	$MKIMAGE $OEM_DIR $ROCKDEV/oem.img $RK_OEM_FS_TYPE
else
	echo -e "\e[31m error: create oem image fail! \e[0m"
fi

if [ -d $USER_DATA_DIR ]
then
	$MKIMAGE $USER_DATA_DIR $ROCKDEV/userdata.img $RK_USERDATA_FS_TYPE
else
	echo -e "\e[31m error: $USER_DATA_DIR not found! \e[0m"
fi

if [ -f $UBOOT_IMG ]
then
        echo -n "create uboot.img..."
        cp -a $UBOOT_IMG $ROCKDEV/uboot.img
        echo "done."
else
        echo -e "\e[31m error: $UBOOT_IMG not found! \e[0m"
fi

if [ -f $TRUST_IMG ]
then
        echo -n "create trust.img..."
        cp -a $TRUST_IMG $ROCKDEV/trust.img
        echo "done."
else
        echo -e "\e[31m error: $TRUST_IMG not found! \e[0m"
fi

if [ -f $LOADER ]
then
        echo -n "create loader..."
        cp -a $LOADER $ROCKDEV/MiniLoaderAll.bin
        echo "done."
else
	echo -e "\e[31m error: $LOADER not found,or there are multiple loaders! \e[0m"
	rm $LOADER
fi

if [ -f $BOOT_PATH ]
then
	echo -n "create boot.img..."
	cp -a $BOOT_IMG $ROCKDEV/boot.img
	echo "done."
else
	echo -e "\e[31m error: $BOOT_IMG not found! \e[0m"
fi

echo -e "\e[36m Image: image in rockdev is ready \e[0m"

