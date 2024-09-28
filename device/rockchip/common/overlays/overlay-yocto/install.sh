#!/bin/bash -e

TARGET_DIR="$1"
[ "$TARGET_DIR" ] || exit 1

OVERLAY_DIR="$(dirname "$(realpath "$0")")"
SDK_DIR="${SDK_DIR:-$(realpath "$OVERLAY_DIR/../../../../..")}"

if [ -x "$TARGET_DIR/usr/bin/weston" ]; then
	echo "Installing weston overlay: $OVERLAY_DIR to $TARGET_DIR..."
	rsync -av --chmod=u=rwX,go=rX "$OVERLAY_DIR/" "$TARGET_DIR/" \
		--exclude="$(basename "$(realpath "$0")")"

	echo "Installing Rockchip test scripts to $TARGET_DIR..."
	rsync -av --chmod=u=rwX,go=rX "$SDK_DIR/external/rockchip-test/" \
		"$TARGET_DIR/rockchip-test/" \
		--include="camera/" --include="video/" --exclude="/*"
fi
