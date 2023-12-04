#!/bin/bash

########################################
#              Chip Info               #
########################################
# Target arch
export RK_ARCH=arm
# target chip
export RK_TARGET_PRODUCT=rv1108

########################################
#             Board Info               #
########################################
#Target Board Version
export RK_TARGET_BOARD_VERSION=minievb-v10
# Set flash type. support <emmc, nand, nor>
export RK_STORAGE_TYPE=nor
# Set depth camera
export RK_HAS_DEPTH_CAMERA=n

########################################
#           Buildroot Config           #
########################################
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rv1108_lock_defconfig

########################################
#            Rootfs Config             #
########################################
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=cpio.lz4

########################################
#            Loader Config             #
########################################
# Set loader config
export RK_LOADER_BUILD_TYPE=all
export RK_LOADER_POWER_HOLD_GPIO_GROUP=none
export RK_LOADER_POWER_HOLD_GPIO_INDEX=none
export RK_LOADER_EMMC_TURNING_DEGREE=0
export RK_LOADER_BOOTPART_SELECT=0
export RK_LOADER_PREISP_EN=0

########################################
#            Kernel Config             #
########################################
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rv1108_defconfig
# Kernel dts
export RK_KERNEL_DTS=rv1108-${RK_TARGET_BOARD_VERSION}

########################################
#           Userdata Config            #
########################################
# Set userdata config
export RK_USERDATA_FILESYSTEM_TYPE=jffs2
export RK_USERDATA_FILESYSTEM_SIZE=6M

########################################
#             Root Config              #
########################################
# Set root data config
export RK_ROOT_FILESYSTEM_TYPE=jffs2
export RK_ROOT_FILESYSTEM_SIZE=6M

########################################
#            Firmware Config           #
########################################
# setting.ini for firmware
export RK_SETTING_INI=setting.ini

########################################
#            Build Config              #
########################################
# Build jobs
export RK_JOBS=12

########################################
#              APP Config              #
########################################
#Set ui_resolution
export RK_UI_RESOLUTION=1280x720
# Set face detection parameter
export RK_FACE_DETECTION_WIDTH=720
export RK_FACE_DETECTION_HEIGHT=1280
export RK_FACE_DETECTION_OFFSET_X=-50
export RK_FACE_DETECTION_OFFSET_Y=-50
# Set UVC source
export RK_UVC_USE_SL_MODULE=n
# Set first start application
export RK_FIRST_START_APP="lock_app system_manager face_service uvc_app"
