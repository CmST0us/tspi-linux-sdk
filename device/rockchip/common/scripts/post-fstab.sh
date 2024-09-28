#!/bin/bash -e

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

FSTAB="$TARGET_DIR/etc/fstab"

fixup_root()
{
	echo "Fixing up rootfs type: $1"

	FS_TYPE=$1
	sed -i "s#\([[:space:]]/[[:space:]]\+\)\w\+#\1${FS_TYPE}#" "$FSTAB"
}

del_part()
{
	echo "Deleting partition: $1 $2"

	SRC="$1"
	MOUNTPOINT="$2"

	# Remove old entries with same mountpoint
	sed -i "/[[:space:]]${MOUNTPOINT//\//\\\/}[[:space:]]/d" "$FSTAB"

	if [ "$SRC" != tmpfs ]; then
		# Remove old entries with same source
		sed -i "/^${SRC//\//\\\/}[[:space:]]/d" "$FSTAB"
	fi
}

fixup_part()
{
	del_part $@

	echo "Fixing up partition: ${@//:/ }"

	SRC="$1"
	MOUNTPOINT="$2"
	FS_TYPE="$3"
	MOUNT_OPTS="$4"
	PASS="$5"

	# Append new entry
	echo -e "$SRC\t$MOUNTPOINT\t$FS_TYPE\t$MOUNT_OPTS\t0 $PASS" >> "$FSTAB"

	mkdir -p "$TARGET_DIR/$MOUNTPOINT"
}

fixup_basic_part()
{
	echo "Fixing up basic partition: $@"

	FS_TYPE="$1"
	MOUNTPOINT="$2"
	MOUNT_OPTS="${3:-defaults}"

	fixup_part "$FS_TYPE" "$MOUNTPOINT" "$FS_TYPE" "$MOUNT_OPTS" 0
}

fixup_device_part()
{
	echo "Fixing up device partition: $@"

	DEV="$1"

	# Dev is either <name> or /dev/.../<name> or <UUID|LABEL|PARTLABEL>=xxx
	[ "$DEV" ] || return 0
	echo $DEV | grep -qE "^/|=" || DEV="PARTLABEL=$DEV"

	MOUNTPOINT="${2:-/${DEV##*[/=]}}"
	FS_TYPE="${3:-ext4}"
	MOUNT_OPTS="${4:-defaults}"

	fixup_part "$DEV" "$MOUNTPOINT" "$FS_TYPE" "$MOUNT_OPTS" 2
}

echo "Fixing up /etc/fstab..."

mkdir -p "$TARGET_DIR/etc"
touch "$FSTAB"

case "$RK_ROOTFS_TYPE" in
	ext[234])
		fixup_root "$RK_ROOTFS_TYPE"
		;;
	*)
		fixup_root auto
		;;
esac

fixup_basic_part proc /proc
fixup_basic_part devtmpfs /dev
fixup_basic_part devpts /dev/pts mode=0620,ptmxmode=0666,gid=5
fixup_basic_part tmpfs /dev/shm nosuid,nodev,noexec
fixup_basic_part sysfs /sys
fixup_basic_part configfs /sys/kernel/config
fixup_basic_part debugfs /sys/kernel/debug
fixup_basic_part pstore /sys/fs/pstore

if [ "$POST_OS" = recovery ]; then
	fixup_device_part /dev/sda1 /mnt/udisk auto
	fixup_device_part /dev/mmcblk1p1 /mnt/sdcard auto
fi

for idx in $(seq 1 "$(rk_extra_part_num)"); do
	DEV="$(rk_extra_part_dev $idx)"
	MOUNTPOINT="$(rk_extra_part_mountpoint $idx)"
	FS_TYPE="$(rk_extra_part_fstype $idx)"

	# No fstab entry for built-in partitions
	if rk_extra_part_builtin $idx; then
		del_part "$DEV" "$MOUNTPOINT" "$FS_TYPE"
		continue
	fi

	fixup_device_part "$DEV" "$MOUNTPOINT" "$FS_TYPE" \
		"$(rk_extra_part_options $idx)"
done
