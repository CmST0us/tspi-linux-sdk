#!/bin/bash

# Target arch
export RK_ARCH=arm
# build idblock.bin and update SPL
export RK_IDBLOCK_UPDATE_SPL=true
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rv1126-spi-nor-tb
# Loader update spl from Uboot
export RK_LOADER_UPDATE_SPL=true
# Uboot image format type: fit(flattened image tree)
export RK_UBOOT_FORMAT_TYPE=fit
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rv1126_defconfig
# kernel defconfig fragment
export RK_KERNEL_DEFCONFIG_FRAGMENT=rv1126-tb.config
# Kernel dts
export RK_KERNEL_DTS=rv1126-evb-ddr3-v12-tb-spi-nor
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/Image
# parameter for GPT table
export RK_PARAMETER=parameter-spi-nor-tb-32M.txt
# Buildroot config
export RK_CFG_BUILDROOT=
# Recovery config
export RK_CFG_RECOVERY=
# ramboot idt config
export RK_RECOVERY_FIT_ITS=boot-tb.its
# ramboot config
export RK_CFG_RAMBOOT=rockchip_rv1126_evb_spi_nor_tb
# Pcba config
export RK_CFG_PCBA=
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rv1126_rv1109
# Set rootfs type, including squashfs jffs2
# export RK_ROOTFS_TYPE=squashfs
# rootfs image path
# export RK_ROOTFS_IMG=rockdev/rootfs.${RK_ROOTFS_TYPE}
# Set ramboot image type
export RK_RAMBOOT_TYPE=ROMFS
# Set oem partition type, including jffs2 squashfs
export RK_OEM_FS_TYPE=
# Set userdata partition type, including jffs2, fat, squashfs
export RK_USERDATA_FS_TYPE=
#OEM config
export RK_OEM_DIR=
#userdata config, if not define this, system will format by RK_USERDATA_FS_TYPE
export RK_USERDATA_DIR=
#misc image
export RK_MISC=
#choose enable distro module
export RK_DISTRO_MODULE=
# Define package-file for update.img
export RK_PACKAGE_FILE=rv1126-package-file-spi-nor-tb
