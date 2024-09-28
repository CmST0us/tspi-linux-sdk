#!/bin/bash -e

POST_OS_DISALLOWED="recovery pcba"

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

echo "Preparing extra partitions..."

for idx in $(seq 1 "$(rk_extra_part_num)"); do
	MOUNTPOINT="$(rk_extra_part_mountpoint $idx)"
	OUTDIR="$(rk_extra_part_outdir $idx)"

	rk_extra_part_prepare $idx "$TARGET_DIR/$MOUNTPOINT"
	rk_extra_part_builtin $idx || continue

	echo "Merging $OUTDIR into $TARGET_DIR/$MOUNTPOINT (built-in)"
	rsync -a "$OUTDIR/" "$TARGET_DIR/$MOUNTPOINT"
done
