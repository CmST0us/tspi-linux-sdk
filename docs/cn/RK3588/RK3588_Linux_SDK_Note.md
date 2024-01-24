# RK3588 Linux SDK Note

---

**Contents**

[TOC]

---

## rk3588_linux_release_v1.1.1_20230520.xml Note

The main update list is as follows:

1 **Update device/rockchip**

- buildroot/yocto installs Chinese fonts by default
- recovery does not install additional overlay
- Fixed hostname exception after dynamically switching rootfs
- Add dependency check and installation prompt
- Add repair owner permission
- Fix the problem that repeated compilation of yocto post-rootfs does not execute

2 **Update recovery**

- Repair press the recovery button to reset to enter the system, nothing is displayed
- U disk upgrade startup support
- Solve the problem that the first upgrade of the SD card starts abnormally
- Fix userdata partition unmount failure

3 **Update Debian**

- Solve the problem of cheese app recording screen freeze
- Support xfce4 power management configuration
- There are hidden problems in updating system permissions
- Update rkwifibt to solve problems related to switch or sleep wake-up
- Update adb to add some feature support
- Update mpp and gstreamer-rockchip
- Update rockchip-test to V2.1
- Solve the abnormal double-click problem of Debian desktop icons

4 **Update Buildroot**

- Optimize rknpu2 compilation configuration
- Add support for setting the location of Gstreamer glimagesink/xvimagesink plugin
- Fixed crash when Weston destroys dmabuf
- Add power/dictionary pen/sweeping machine and other product configurations
- Added support for external toolchains
- Add support for GCC8.X
- Fix the problem that the video source size may overflow after Gstreamer kmssink scaling
- Add Chromium 111.0.5563.147, support video H265 decoding
- Update Frecon, solve zoom and switch VT1 support
- Update rockit to v1.7.4
- Add support for lvgl demo

5 **Update rkbin**

- Adjust the pvtpll configuration of cpu/gpu/npu according to the correlation chip test results
- Increase chip version judgment
- Update RK3588 lp5 mr configuration \ improve the stability of hdmirx related functions \ add ddr spread spectrum mode / optimize boot time.

6 **Update Kernel**

- Solve the problem that the rk3588 hdmiphy lane skew is too large
- Update rknpu driver to V0.8.8
- Solve isp switch abnormal error
- System panic after opening KASAN with Logo buf non-PAGE aligned

7 **Update Uboot**

- fastboot: Burning more than 4G firmware causes the system to fail to start
- System panic after opening KASAN with Logo buf non-PAGE aligned

## rk3588_linux_release_v1.1.0_20230420.xml Note

The main update list is as follows:

### SDK update main core component versions

- Kernel upgrade from 5.10.110 to 5.10.160
- Update Debian to 11.6
- Update Yocto to 4.0.9
- Update Buildroot to November 2021
- Weston updated to 11.0.1
- Gstreamer updated to 1.22

### SDK optimization and adjustment

- Reconstruct SDK configuration compilation mechanism
- Adjusting the compilation mechanism of the wifibt module
- Import a new version of Camera rkaiq to optimize its functionality and performance

### SDK New features

- Added Linux headers support, making it convenient for third-party applications without the need for kernel compilation and debugging
- Added yocto support for x11
- Added gst mpp support for av1
- Added gst mpp support for afbc encoding
- New support for OTA differential function
- New UDL support
- Added adaptation support for libcamera
- New rk3588 venc support for dynamic voltage regulation to improve stability

### SDK main fix issues

- Fix ubi format partition packaging and mounting issues
- Fix recovery mount partition exception
- Fix [webgl](https://webglsamples.org/aquarium/aquarium.html) Flash screen issue
- Fixed the issue of resetting the time to 0 after standby wake-up
- Fix fiq debugger driver, serial port RX interference, resulting in system stuck
- Fixed a low probability of error after starting KASAN: KASAN: use after free in rga_ job_ next
- Support for addressing Weston touch related configurations
- Solve HDMI/MIPI plug and display issues
- Solve the problem of PDM recording channel confusion
- Resolve the issue of playback noise caused by RK809 · RK817-pdm recording
- Solving the unexpected startup of PMU MCU during sleep of rk3588 may cause ddr data to be overwritten
- Improve the stability of RK3588 DDR

## rk3588_linux_release_v1.0.3_20220920.xml Note

- Upgrade bifrost DDK from g12p0-01eac0 to g13p0-01eac0
- Upgrade NPU verison to V1.4.0
- Refactor rkwifibt code
- Update Weston to support to set cursor size and launchers
- Add pm-utils for PowerManager
- Use chromium R88 by default
- Update some packages to buildroot upstream version
- Update Linux_Upgrade_Tool to v2.17
- Update UEFI
- Update audio/crypto/sata/PCIe/usb/rockit/gstreamer/rga... docs

## rk3588_linux_release_v1.0.2_20220820.xml Note

- Remove libglCompositor/avb repositories
- Add rockit/librkcrypto repositories
- Fix a few download/extract errors
- Upgrade Weston version to 10.0.1 and improve its stability
- Add support for buildroot X11 related functions
- Add support for Linux enlightment Wayland desktop
- Upgrade kernel to 5.10.110
- Update the chromium browser version of Debian to R101
- Update Linux_Upgrade_Tool, RKDevTool and SDDiskTool
- Improve HDMI compatibility and HDMI-IN stability
- Upgrade GStreamer of builderroot to 1.20.3 to improve audio and video compatibility
- Add gamma support for xserver and fix the problem of multi touch
- Update loader to improve chip stability
- Update NPU and fix memory leakage
- Update MPP to improve compatibility
- Update docs

## rk3588_linux_release_v1.0.1_20220620.xml Note

- Fixes Secureboot function issue
- Fixes SD boot and upgrade issue
- Update docs include secureboot,pinctrl,display,AVl and so on
- Update kernel to fix some issues
- Add RK3588M Socs to support
- Add RK3588 evb7 to support single pmic hardware design

## rk3588_linux_release_v1.0.0_20220520.xml Note

- Update all the projects with lastest
- Update kernel/uboot/rkbin to improve the chips stability
- Update documents and tools to make the SDK more better
- Fixes some bugs on buildroot weston10
- Fixes the multivideo hang issue
- Fixes the suspend to resume issues
- Fixes the kmssink/rksimageink plugins on gstreamer
- Fixes the bluthooth on/off during the suspend to resume
- Fixes the pcie-ssd performance with write/read
- Fixes the userdata address in parameter.txt
- Fixes the cheese app on debian
- Fixes the permission with pulseaudio
- Improve the video decode performance with mpp fast mode
- Improve the Audio/Video compatibility
- Upgrade RKNN SDK to v1.3.0
- Upgrade RKAIQ from v3.0x8.7 to v3.0x8.8
- Upgrade Debian version from 11.2 to 11.3
- Reduce the debian rootfs size from 5.5G to 3.2G

## rk3588_linux_beta_v0.1.1_20220421.xml Note

- Support midi and fluidsynth format for gstreamer
- Convert github git:// to https:/ on buildroot
- Support mpp fast-mode property

## rk3588_linux_beta_v0.1.0_20220414.xml Note

```
- The first beta version
```

## rk3588_linux_alpha_v0.0.1_20220115.xml Note

```
- The first alpha version
```
