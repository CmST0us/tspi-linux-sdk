#!/bin/bash

# Target chip
export RK_CHIP=RV1126
# Target arch
export RK_ARCH=arm
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rv1126
# Uboot defconfig fragment, config rk-nand.config if sdcard upgrade
export RK_UBOOT_DEFCONFIG_FRAGMENT=rk-nand.config
# Loader update spl from Uboot
export RK_LOADER_UPDATE_SPL=true
# Uboot image format type: fit(flattened image tree)
export RK_UBOOT_FORMAT_TYPE=fit
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rv1126_defconfig
# Kernel defconfig fragment
export RK_KERNEL_DEFCONFIG_FRAGMENT=rv1126-slc-nand.config
# Kernel dts
export RK_KERNEL_DTS=rv1126-evb-ddr3-v12-spi-nand
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image format type: fit(flattened image tree)
export RK_KERNEL_FIT_ITS=boot.its
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/zImage
# parameter for GPT table
export RK_PARAMETER=parameter-fit-nand-256M.txt
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rv1126_rv1109_spi_nand
# Recovery config
export RK_CFG_RECOVERY=rockchip_rv1126_rv1109_spi_nand_recovery
# Recovery image format type: fit(flattened image tree)
export RK_RECOVERY_FIT_ITS=boot4recovery.its
# ramboot config
export RK_CFG_RAMBOOT=
# Pcba config
export RK_CFG_PCBA=
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rv1126_rv1109
# Set rootfs type, including ext2 ext4 squashfs ubi
export RK_ROOTFS_TYPE=ubi
# rootfs image path
export RK_ROOTFS_IMG=rockdev/rootfs.${RK_ROOTFS_TYPE}
# Set ramboot image type
export RK_RAMBOOT_TYPE=
# Set oem partition type, including ext2 squashfs
export RK_OEM_FS_TYPE=ubi
# Set userdata partition type, including ext2, fat
export RK_USERDATA_FS_TYPE=ubi
#OEM config
export RK_OEM_DIR=
#userdata config, if not define this, system will format by RK_USERDATA_FS_TYPE
export RK_USERDATA_DIR=
#
# Set ubifs page size, 2048(2KB) or 4096(4KB)
# Option.
# export RK_UBI_PAGE_SIZE=2048
#
# Set ubifs block size, 0x20000(128KB) or 0x40000(256KB)
# Option.
# export RK_UBI_BLOCK_SIZE=0x20000
#
# Set userdata partition size (byte) if define RK_USERDATA_DIR
# MUST, if userdata partition is grow partition.
# export RK_USERDATA_PARTITION_SIZE=0x02760000
#
# Set oem partition size (byte)
# Option. if not set, it will get from parameter auto.
# export RK_OEM_PARTITION_SIZE=0x6400000
#
#misc image
export RK_MISC=blank-misc.img
# Define pre-build script for this board
export RK_BOARD_PRE_BUILD_SCRIPT=app-build.sh
# Define package-file for update.img
export RK_PACKAGE_FILE=rv1126-package-file-spi-nand
