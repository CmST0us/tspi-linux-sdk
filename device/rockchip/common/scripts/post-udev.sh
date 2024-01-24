#!/bin/bash -e

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

[ -n "$RK_ROOTFS_UDEV_RULES" ] || exit 0

cd "$SDK_DIR"

mkdir -p "$TARGET_DIR/lib/udev/rules.d"
for rule in external/rkscript/*.rules; do
	echo $rule | grep -vq usbdevice || continue

	echo "Installing udev rule: $rule"
	install -m 0644 $rule "$TARGET_DIR/lib/udev/rules.d/"
done
