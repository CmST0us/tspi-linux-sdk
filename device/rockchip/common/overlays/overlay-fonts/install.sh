#!/bin/bash -e

TARGET_DIR="$1"
[ "$TARGET_DIR" ] || exit 1

OVERLAY_DIR="$(dirname "$(realpath "$0")")"
cd "$OVERLAY_DIR"

for f in *.tar; do
	echo "Installing extra font(${f%.tar}) to $TARGET_DIR..."
	tar xf "$f" -C "$TARGET_DIR"
done
