#!/bin/bash -e

[ -z "$DEBUG" ] || set -x

PATCH_DIR="$(dirname "$(realpath "$0")")"

SDK_DIR="$(realpath "${1:-"${SDK_DIR:-"$PATCH_DIR/../../../../../"}"}")"

echo "Applying all patches from $PATCH_DIR to $SDK_DIR"

cd "$PATCH_DIR"
for s in $(find . -name apply-patches.sh); do
	DIR="$(realpath "$SDK_DIR/$(dirname $s)")"
	echo "Applying patches to $DIR"
	cd "$DIR" && "$PATCH_DIR/$s"
	echo "Done"
done
