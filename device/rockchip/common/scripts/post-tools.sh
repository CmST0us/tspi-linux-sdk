#!/bin/bash -e

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

[ -n "$RK_ROOTFS_PREBUILT_TOOLS" ] || exit 0

echo "Installing prebuilt tools..."

mkdir -p "$TARGET_DIR/usr/local/bin/"
rsync -av --chmod=u=rwX,go=rX --exclude=adbd \
	"$RK_DATA_DIR/tools/" "$TARGET_DIR/usr/local/bin/"
ln -sf perf-$RK_KERNEL_VERSION_REAL "$TARGET_DIR/usr/local/bin/perf"
