#!/bin/bash

# Target arch
export RK_ARCH=arm
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rk3308-aarch32
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rk3308_linux_aarch32_debug_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk3308b-evb-amic-v10-aarch32
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/zImage
# parameter for GPT table
export RK_PARAMETER=parameter-32bit.txt
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk3308_b_32_release
# Recovery config
export RK_CFG_RECOVERY=rockchip_rk3308_recovery
# ramboot config
export RK_CFG_RAMBOOT=
# Pcba config
export RK_CFG_PCBA=rockchip_rk3308_pcba
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rk3308
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=squashfs
# rootfs image path
export RK_ROOTFS_IMG=rockdev/rootfs.${RK_ROOTFS_TYPE}
# Set oem partition type, including ext2 squashfs
export RK_OEM_FS_TYPE=ext2
# Set userdata partition type, including ext2, fat
export RK_USERDATA_FS_TYPE=ext2
#OEM config: /oem/dueros/aispeech-6mic-64bit/aispeech-2mic-64bit/aispeech-4mic-32bit/aispeech-2mic-32bit/aispeech-2mic-kongtiao-32bit/iflytekSDK/CaeDemo_VAD/smart_voice
export RK_OEM_DIR=oem
#userdata config
export RK_USERDATA_DIR=userdata_empty
MIC_NUM=6
#misc image
export RK_MISC=wipe_all-misc.img
#choose enable distro module
export RK_DISTRO_MODULE=
# loader name
export RK_LOADER_NAME=*_loader_uart4_v*.bin
