#!/bin/bash

# Target arch
export RK_ARCH=arm
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rk3126
# SPL INI
export RK_SPL_INI_CONFIG=RK3126MINIALL_WO_FTL.ini
# Uboot defconfig fragment,config rk-nand.config
export RK_UBOOT_DEFCONFIG_FRAGMENT=rk312x-slc-nand.config
# Trust ini config
export RK_TRUST_INI_CONFIG=RK3126TOS_LADDR.ini
# Uboot size
export RK_UBOOT_SIZE_CONFIG=1024\ 2
# Trust size
export RK_TRUST_SIZE_CONFIG=1024\ 2
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rk3126_linux_defconfig
# Kernel defconfig fragment
export RK_KERNEL_DEFCONFIG_FRAGMENT=rk312x-slc-nand.config
# Kernel dts
export RK_KERNEL_DTS=rk3126c-evb-ddr3-v10-linux-slc
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/zImage
# parameter for GPT table
export RK_PARAMETER=parameter-buildroot-slc.txt
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk3126c
# Recovery config
export RK_CFG_RECOVERY=rockchip_rk312x_recovery
# ramboot config
export RK_CFG_RAMBOOT=
# Pcba config
export RK_CFG_PCBA=rockchip_rk3126c_pcba
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rk3126c
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=ubi
# rootfs image path
export RK_ROOTFS_IMG=rockdev/rootfs.${RK_ROOTFS_TYPE}
# Set oem partition type, including ext2 squashfs
export RK_OEM_FS_TYPE=ubi
# Set userdata partition type, including ext2, fat
export RK_USERDATA_FS_TYPE=ubi
#OEM config
export RK_OEM_DIR=oem_normal
#userdata config
export RK_USERDATA_DIR=userdata_normal
#misc image
export RK_MISC=wipe_all-misc.img
#choose enable distro module
export RK_DISTRO_MODULE=

export RK_USERDATA_PARTITION_SIZE=0xB400000
export RK_OEM_PARTITION_SIZE=0x1400000
