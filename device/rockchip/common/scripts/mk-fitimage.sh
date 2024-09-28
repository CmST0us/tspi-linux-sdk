#!/bin/bash -e

TARGET_IMG="$1"
ITS="$CHIP_DIR/$2"
KERNEL_IMG="$3"
RAMDISK_IMG="$4"
KERNEL_DTB="$RK_KERNEL_DTB"
RESOURCE_IMG=kernel/resource.img

if [ ! -f "$ITS" ]; then
	echo "$ITS not exists!"
	exit 1
fi

TMP_ITS=$(mktemp)
cp "$ITS" "$TMP_ITS"

if [ "$RK_SECURITY" ]; then
	echo "Security boot enabled, removing uboot-ignore ..."
	sed -i "/uboot-ignore/d" "$TMP_ITS"
fi

sed -i -e "s~@KERNEL_DTB@~$(realpath -q "$KERNEL_DTB")~" \
	-e "s~@KERNEL_IMG@~$(realpath -q "$KERNEL_IMG")~" \
	-e "s~@RAMDISK_IMG@~$(realpath -q "$RAMDISK_IMG")~" \
	-e "s~@RESOURCE_IMG@~$(realpath -q "$RESOURCE_IMG")~" "$TMP_ITS"

rkbin/tools/mkimage -f "$TMP_ITS"  -E -p 0x800 "$TARGET_IMG"
