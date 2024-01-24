# RK356X Linux SDK Note

---

**Versions**

[TOC]

---
## rk356x_linux_release_v1.3.0_20220620.xml Note

**Buildroot**:

```
- Update buildroot 2018.02-rc3
	* - Fix sd upgrade and boot issue with SDDiskTool
	* - Switch weston to dispaly as launcher
	* - Support eglfs
	* - Support AFBC
	* - Support vulkan and vkmark
	* - Update weston to fix some issues
	* - Update gstreamer1 to fix hang issue
	* - Update configure to disable RGA by default
	* - Update gstreamer1 to fix some issue with v4l2src plugin
	* - Update rockchip-test to match new version
	* - Upgrade wayland to buildroot upstream's 1.20.0
	* - Upgrade to Chromium-wayland to 101.0.4951.54
	* - Upgrade RK356X NPU to v1.3.0
	...
```

**Debian**:

```
- Update Debian10
	* - Update the font to chinese by default
	* - Update mirrors.ustc.edu.cn for source.list
	* - Update mpp/gst-rkmpp/xserver
	* - Update rockchip-test
	* - Update gstreamer to fix the format issues
	* - Update Powermanager for s2r
	* - Fixes cheese app issues
	* - Add rktoolkit and partition init for recovery
	* - Reduce the rootfs size
    ...
```

**Kernel**:

```
- Update Kernel to 4.19.232
    * - Update pcie3phy firmware for RK3568
    * - Fixes the GPU OPP error with stress tests
    ...
```

**Yocto**:

```
- Update Yocto to 3.4.1
	* - Support Chromium to 101.0.4951
	* - Bump xserver/v4l-utils/v4l-mpp/gst-mpp/mpp
	* - Update rkwifibt-firmware and rockchip-libmali
```

**Other**:

```
- Update rkbin/u-boot/tools/docs/gstreamer-rockchip/mpp...
```

## rk356x_linux_release_v1.2.4_20220418.xml Note

**Buildroot**:

```
- update buildroot 2018.02-rc3
	* - Gstreamer upgrade to 1.20.0 version
	* - Bump glibc to 2.34
	* - Bump weston to 10.0.0
	...
```

**Debian**:

```
- update Debian10
	* - 2775061 ubuntu-build-service: remove hostapd and gstreamer1.0-libav
	* - 4da1c99 overlay: fix the ssh service
	* - 2a5f79c packages: update mpp/gst-rkmpp/xserver
	* - df16049 scripts: add post-build.sh to handle fstab
	* - e15ea61 overlay-debug: delete unused tests
	* - 1cc248a packages: update rkaiq for rk356x
	* - 966eed2 overlay-debug: update rockchip-test
        ...
```

**Kernel**:

```
- update Kernel to 4.19.232
        * - Suport RK630
        * - Update isp to v1.8.0
        * - Fix USB some issues
        * - Update rga
        ...
```

**Yocto**:

```
- Bump yocto to 3.4.1
```

- Update rkbin/u-boot/aiserver/rkwifibt...

## rk356x_linux_release_v1.2.3_20220108.xml Note

**Buildroot**:

```
- update buildroot 2018.02-rc3
	* - glmark2 upgrade to 2021.02 version
	* - Support AFBC for kmssink and waylandsink
	* - Fix a memory leak in window create/destroy on qt5wayland
	* - Support NV12_10 and NV16 format
	...
```

**Debian**:

```
- update Debian10
	* - ecfd77c packages: update gstreamer for afbc
	* - 4f78d66 packages: armhf: Update gstreamer and gstreamer plugins
	* - 776346f overlay-debug: update scripts
	* - 33934b6 mk-rootfs-buster.sh: remove the typo
	* - a438f20 packages: update gst-rkmpp/mpp/libdrm-cursor/xserver
	* - ed04efc packages: Update gstreamer and gstreamer plugins
	* - 41a6ac8 mk-rootfs-buster.sh: add some packages
	* - 9d2aac4 overlay-firmware: remove unused wifi tools
	* - 93c150e overlay-debug: add rockchip_test
	* - a792a28 overlay-debug: update ddr test tools
	* - 18985be overlay: update services
	...
```

**external**:

```
- update camera_engine_rkaiq
	* - c1b0f18 rkaiq_3A_server: workaround: always enable readback
	* - e758b50 update aiq to version v2.0x60.1

- update mpp
	* - 693720fd ([iep2]: Disable test log
	* - e85c28ab [vp9d]: Fix ref frame pointer not free issue
	* - f94ae6d0 [vepu580]: Add YUV444 support for vepu580
	* - 5dec7c0b [hal_h265e]: Format file from dos to unix
	* - be46d787 [hal_hevc580]: Add frame offset x y set
	...

- update gstreamer-rockchip
	* - 9db2606 rkximage: Fix pitch error for YUV420_8BIT
	* - 652bf72 HACK: rkximage: Fix aligning error for AFBC
	* - 983a1e1 mppdec: Adjust crop size based on MPP's offsets
	* - b0d0fc5 HACK: mppdec: Avoid copying output buffer in make_writable() while shared
	* - e2e2491 mppdec: Honor interlace mode changing
	* - b35866b mppdec: Support crop-rectangle property
	...
```

**Kernel**:

```
- update Kernel to 4.19.219
	* - 82957dba3977 drm/rockchip: vop2: add support DRM_FORMAT_YUYV for RK356x Cluster
	* - 6b8a1e9f8ebf drm/rockchip: dev_ebc: release version v2.26
	* - 3987669c73ce media: i2c: add new camera sensor gc030a
	* - 61cf54704b72 media: rockchip: isp: improve snapshot feature(tb in RISC-V)
	...
```

**Uboot**:

```
- update rkbin
	* - c7a0111 rk3568: bl32: fix pack failure
	* - 0419aef rk3568: bl32: update version to v2.01
	...
```

**Tools**:

```
- update tools
	* - 1a32bc7 tools: linux: update Linux_Upgrade_Tool to v2.1
	* - 00ad7ea tools: windows: update RKDevTool to v2.91
	...
```

## rk356x_linux_release_v1.2.2_20211205.xml Note

**Buildroot**:

```
- update buildroot 2018.02-rc3
	* - add arm32 rknpu2 test mode and demo
	* - gstreamer1: gstpad: Add 1 sec timeout for activation
	* - gst1-plugins-base: playbin2: Fix deadlock when hooking about-to-finish signal
	* - qt5multimedia: Support choosing playbin2 and playbin3
	...
```

**external**:

```
- update mpp
	* - [mpp_impl]: modify dump frame default size
	* - [mpp_dec]: Optimize sort pts function
	* - [mpp_list]: Add list_sort func
	* - [mpp_enc]: Fix rotation case GET_CFG mismatch
	* - [h264e_vepu]: add cfg to disable mb rc
	* - [hal_task]: Remove hal_enc_task.h
	* - [mpp_hal]: Remove enc define from HalTaskInfo
	* - [mpp_cluster]: Change callback return type
	...

- update gstreamer-rockchip
	* - rockchipmpp: Add mppvpxalphadecodebin element
	* - mppenc: Add mpph265enc
	* - mppenc: Fix unbalance stream unlock
	*- mppdec: Unlock stream when doing RGA conversion
	* - rkximage/mppdec: Support NV12_10(AFBC)
	* - rkximage: Disable colorkey by default
	...
```

**kernel**:

```
- update Kernel4.19
	* - video: rockchip: mpp: rkvdec2: setup link mode clk
	* - video: rockchip: mpp: Fix mpp_iommu_refresh crash issue
	...
```

## rk356x_linux_release_v1.2.0_20210930.xml Note

**app**:

```
- update qsetting
	* - qsetting: wifibt: fix build
	* - qtbt:Solve the bug of incomplete Bluetooth display

- update rkaiq_tool_server
	* - media pipeline: only link for selected sensor
```

**external**:

```
- update mpp
	* - [hal_task]: Remove unused variable
	* - [mpp_dec]: Optimize decoder flow
	* - [mpp_lock]: Add gcc atomic macro define
	* - [h265d_parser]: Fix h265d parser crash issue
	* - [mpp_meta]: Use macro to generate code
	* - [mpp_meta]: Add performance test case
	* - [mpp_meta]: Improve performance
	* - [mpp_dec]: Fix double free issue
	...

- update gstreamer-rockchip
	* - mppenc: Improve format and alignment supports
	* - mppdec: Provide RGA formats only when available
	* - mppdec: Add RGB/BGR formats
	* - mppjpegenc: Update size limits
	* - mppenc: Simplify caps
	* - mppvideodec: Delay discarding frames for some broken videos
	* - mppvideodec: Drop extra MPP frame in I4O2 deinterlaced mode
	* - mppvideodec: Rule out YUV444 for H264
	* - rkximage: Fix colorkey setting issue
	* - mppdec: Use alignment 2 for strides when doing RGA conversion
	* - rkximage: Support disabling vsync
	* - rkximage: Update colorkey prop defination
	* - mppjpegdec: Drop PP format NV16
	* - mppdec: Support setting prefered output format
	* - rockchipmpp: Fix RGA RGB16 wrong endian
	* - mppdec: Drop RGB15 and BGR15
	* - Revert "rockchipmpp: Use height as vstride in RGA conversion"
	* - mppjpegdec: Add a sanity check for input video info
	* - rockchipmpp: Fix a few compile warnings
	* - Switch to meson
	* - Remove unused tests/examples

- update libmali
	* - Move scripts and sources to sub directories
	* - Speed up normalizing
	* - debian: Sort targets
	* - debian: Force enabling wrappers
	* - libmali: px30, 3326: add libs of libmali-bifrost-g31-g2p0-only-cl.so of g2p0-01eac0-8
	* - libmali: px30, 3326: add libs of libmali of g2p0-01eac0-7
	* - libmali optimized for size: 356x: add libmali-bifrost-g52-g2p0-without-cl-dummy-gbm.so of g2p0-01eac0-6
	* - libmali optimized for size: 356x: add libmali-bifrost-g52-g2p0-dummy-gbm.so of g2p0-01eac0-6
	* - meson: Support optimize-level option
	* - debian: Simplify conflicts logic
	* - meson: Don't try to fixup non-existing headers
	* - libmali: 356x: add libmali-bifrost-g52-g2p0-without-cl-dummy-gbm.so of g2p0-01eac0-5

- update linux-rga
	* - build: add .gitignore if build in rga source dir
	* - build: Modify CMakeLists.
	* - drmPrimeHandleToFD add DRM_CLOEXEC | DRM_RDWR flag
	* - build: cmake support compiling with 'buildroot' TARGET.
	* - im2d_api: Fix the error of rgaImDemo fill mode.
	* - Modify the judgment about perpixelAlpha.
	* - Support BGR565/BGRA5551/BGRA4444.
	* - im2d_api: Fix the check error of crop mode.
	* - im2d_api: Fix errors in the blend module.
	* - docs: Modify the wrong format description of RGB and RGBA.
	* - Fix the error of BGR565/5551/4444 format conversion.
	* - Get the version compatible with RGA1.
	* - im2d_api: Remove IM_CROP.

- update camera_engine_rkaiq
	* - update aiq to version v2.0x60.1
	* - add lock for j2s & fix crash on multi camera.
	* - rkaiq_3A_server: start engine in threads
	* - rkaiq_3A_server: get sensor entity name from librkaiq
	* - isp or ispp can be NULL for rkcif media device
	* - CamHwIsp20: fix dvp entity name not matched with driver

- update deviceio_release
	* - devceio_release: update to 20210930
	* - fixed build err in the case of cpp refer c
	* - devceio_release: update to 20210907

- update storage_manager/rknpu/rknn-toolkit2/rockx/rknpu2/isp2-ipc/ipcweb-backend/mediaserver/aiserver/uac_app/common_algorithm/libglCompositor/rkwifibt
```

**Debian**:

```
- update Debian10
	* - mk-rootfs-buster.sh: Error out when source not found
	* - mk-rootfs-buster.sh: Build for arm64 by default
	* - mk-rootfs-buster.sh: Drop unused xserver -dev packages
	* - mk-rootfs-buster.sh: Use apt-get to install local packages
	* - mk-rootfs-buster.sh: Only hold custom local packages
	* - packaegs: update xserver
	* - packages: update libdrm-cursor
	* - overlay-debug: update glmark2
	* - packages: update xserver
	* - overllay: upgrade bifrost-g31 to g2p0
	* - packages: update libmali
	* - packages: update libdrm-cursor
	* - overlay: xorg.conf.d: Add some comments
	* - overlay: fixes the typo for scripts
	* - packages: update xserver
	* - packages/mpp: update mpp
	* - packages: update gst-rkmpp/mpp/rga packages
	* - scripts: the libssl-dev had existed on base package
	* - packages: fixes dri2 pagefilp issue for xserver
	* - mk-rootfs-buster.sh: Only preload libdrm-cursor for X
	* - overlay: enable ASYNC for atomic commit by default
	* - packages: update xserver package
	* - Merge "rkscripts: Don't remove the build dir"
	* - Merge "packages: update rga/mpp/gstreamer-rockchip"
	* - rkscripts: Don't remove the build dir
	* - chromium-x11: Update to 91.0.4472.164
	* - packages: update rga/mpp/gstreamer-rockchip
```

**Yocto**:

```
- update Yocto3.2
	* - linux-rockchip: 4.4: Update color-key patch
	* - linux-rockchip: 4.4: Fix compile error with new GCC
	* - u-boot: Rebase patches
	* - machine: px30: Switch mali to g2p0
	* - Bump BSP package revisions at 2021_10_13
	* - Add drm-cursor
	* - Fix fetching errors for local git sources with detached HEAD
	* - gstreamer-rockchip: Switch to meson build system
```

**Buildroot**:

```
- update buildroot 2018.02-rc3
	* - rknpu: Remove redundant 356x options
	* - rockchip_rk3568_defconfig: fix rknpu2.
	* - qt5wayland: Support window lower() and raise()
	* - configs: add rk3588 nvr defconfig
	* - weston: Use vblank based dynamic repaint window
	* - weston: Improve input device and output  associating
	...
```

**U-boot**:

```
- update U-boot (next-dev)
	* - mtd: spi-nor-ids: Add support for gd25lb512m
	* - dm: sysreset: do optimise
	* - arm: crt0_64.S: disable arm64 SError for usbplug
	* - lib: optee_clientApi: data alignment for get_rkss_version
	* - rockchip: rkimg: support setting NVME as main storage
	* - rockchip: rk3308bs: correct the nand iomux
	...
```

**rkbin**:

```
- update rkbin
	* - rk3566: ddr: update ddr bin to v1.11
	* - rk3568: ddr: update ddr bin to v1.11
	...
```

**kernel**:

```
- update Kernel4.19
	* - video: rockchip: mpp: use dma-buf-cache func
	* - drm: rockchip: do release callback if not define CONFIG_DMABUF_CACHE
	* - clk: rockchip: rk3568: add CLK_SET_RATE_NO_REPARENT flag for clk_gmacx_rx_tx
	* - ASoC: es8311: fixed the dapm route error
	* - phy: rockchip-naneng-usb2: do apb reset during probe
	* - arm64: dts: rockchip: rk3568-linux: enable hdmi_sound with hdmi jack function
	* - arm64: dts: rockchip: rk3568-evb: use "rockchip,hdmi" instead of "simple-audio-card"
	* - mmc: dw_mmc-rockchip: Improve v2 tuning
	* - mmc: dw_mmc-rockchip: Skip all phases bigger than 270 degrees
	* - media: rockchip: ispp: replace iommu detach/attach
	...
```

**docs**

```
- update docs
	* - docs: add ROS2 document and update PCBA and Recovery document
	* - Common/CAMERA: upgrade ISP2x to 20210925
	* - docs: add wifibt avl for linux and update wifibt docs to 20210915
	* - Linux: Multimedia: update Rockchip_Developer_Guide_Linux_RKADK_CN.pdf to v1.2.0
	* - COMMON: update it with inside on 20210922
	* - Linux: Recovery: update DFU upgrade guide document to v1.1.0
	* - docs: update Rockchip_Driver_Guide_VI & Rockchip_Tuning_Guide_ISP20 document;
```

**tools**

```
- update tools
	* - update RKDevTool from V2.84 to V2.86
	* - linux: Linux_Pack_Firmware: add rv1126-package-file-sllock
	* - tools: windows: update ParameterTool to v1.2
	* - linux: Linux_Pack_Firmware: add new package file
	...
```

## rk356x_linux_release_v1.1.0_20210520.xml Note

**Buildroot (2018.02-rc3)**:

```
- Adjust the new buildroot project
- Support buildroot 32 bits for rk356x
- Support RKNN SDK 1.0.0 Version
```

**Debian10 (buster)**:

```
- Use the new debian project
```

**Kernel (4.19)**:

```
- Enable optee by default
- Update USB/DRM/Wireless/Media/Video/Clock driver
```

**docs/tools**:

```
- Use the new docs project
```

**rkbin**:

```
- rk3568/rk3566: bl31: update version to v1.22
- rk3568/rk3566: bl32: update version to v1.05
- rk3568/rk3566: ddr: update ddr bin to v1.07
- rk3568/rk3566: spl: update version to v1.11
- rk356x: loader: update version to v1.08
```

## rk356x_linux_release_v1.0.0_20210410.xml Note

**Buildroot (2018.02-rc3)**:

```
- Upgrade libmali to g2p0
- Upgrade Chromium to 88.0.4324.150
- Support RKNN SDK 0.7 Version
- Update weston to support multi-screen
- Update mpp and gstreamer for mpeg4
- Update rockit
- Fixes qTbase/qt5multimedia/waylandsink/qt5declarative/qt5virtualkeyboard some bugs
- Support lxc and pcl
- Fixes qt5webengine on qt5.15
```

**Yocto**:

```
- Upgrade libmali to g2p0
- Upgrade Chromium to 88.0.4324.1502
```

**Debian10 (buster)**:

```
- Upgrade libmali to g2p0
- Upgrade Chromium to 88.0.4324.1502
- Support multi-screen
- Update rga/libmali/mpp packages
```

**Kernel (4.19)**:

```
- Upgrade Kernel to 4.19.172 from rockchip inside
```

**docs/tools**:

```
- Integrate AVL/DDR/DISPLAY/NVM/PCIe/UART/USB/U-BOOT documents to Common directory
- Update camera and audio documents and directory structure
- Add some rk356x documents
- Update rk_sign_tool to v1.41
- Update RKDevTool to V2.81
- Update SDDiskTool to v1.64
- Update SecureBootTool to v1.99
```

## rk356x_linux_beta_v0.2.0_20210226.xml Note

**Buildroot (2018.02-rc3)**:

```
- Use QT5.14 by default, and support QT5.15
- Upgrade Chromium to 87.0.4280.141
- Fixes qt5webengine HW video decode error on 5.15
- Update weston to fix some bugs
- Update power-key.sh for suspend and resume
- Add rockchip_rk356x_libs_defconfig for small system
```

**Yocto**:

```
- Fixes some issues on Yocto3.2
```

**Debian10 (buster)**:

```
- Fixes some issues on Debian10
```

**Kernel (4.19)**:

```
- Update Kernel from rockchip inside
```

## rk356x_linux_beta_v0.1.0_20210118.xml Note

```
- The first beta version
```

## rk356x_linux_alpha_v0.0.1_20201211.xml Note

```
- The first alpha version
```
