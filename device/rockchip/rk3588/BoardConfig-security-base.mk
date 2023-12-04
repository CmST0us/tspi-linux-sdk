#!/bin/bash
export RK_LOADER_UPDATE_SPL=true

# ramboot config
export RK_CFG_RAMBOOT=rockchip_rk3588_ramboot
# Set ramboot image type
export RK_RAMBOOT_TYPE=CPIO
# Define enable RK SECUREBOOT
export RK_RAMDISK_SECURITY_BOOTUP=true
export RK_SECURITY_OTP_DEBUG=true
export RK_SYSTEM_CHECK_METHOD=DM-V
