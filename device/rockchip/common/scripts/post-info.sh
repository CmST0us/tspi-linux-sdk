#!/bin/bash -e

POST_OS_DISALLOWED="recovery pcba"

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

INFO_DIR="$TARGET_DIR/info"

echo "Adding info dir..."

rm -rf "$INFO_DIR"
mkdir -p "$INFO_DIR"

cd "$SDK_DIR"

yes | ${PYTHON3:-python3} .repo/repo/repo manifest -r \
	-o "$INFO_DIR/manifest.xml" &>/dev/null || true

cat "$RK_CONFIG" | sed "s/\(PASSWORD=\)\".*\"/\1\"********\"/" > \
	"$INFO_DIR/rockchip_config"

cp kernel/.config "$INFO_DIR/config-$RK_KERNEL_VERSION"
cp kernel/System.map "$INFO_DIR/System.map-$RK_KERNEL_VERSION"

EXTRA_FILES=" \
	/etc/os-release /etc/fstab /var/log \
	/tmp/usbdevice.log /tmp/bootanim.log \
	/tmp/resize-all.log /tmp/mount-all.log \
	/proc/version /proc/cmdline /proc/kallsyms /proc/interrupts /proc/cpuinfo \
	/proc/softirqs /proc/device-tree /proc/diskstats /proc/iomem \
	/proc/meminfo /proc/partitions /proc/slabinfo \
	/proc/rk_dmabuf /proc/rkcif-mipi-lvds /proc/rkisp0-vir0 \
	/sys/kernel/debug/wakeup_sources /sys/kernel/debug/clk/clk_summary \
	/sys/kernel/debug/gpio /sys/kernel/debug/pinctrl/ \
	/sys/kernel/debug/dma_buf /sys/kernel/debug/dri \
	"
ln -sf $EXTRA_FILES "$INFO_DIR/"
