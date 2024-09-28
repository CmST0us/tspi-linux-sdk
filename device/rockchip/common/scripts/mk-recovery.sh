#!/bin/bash -e

usage_hook()
{
	echo -e "recovery                          \tbuild recovery"
}

clean_hook()
{
	check_config RK_RECOVERY_CFG || return 0
	rm -rf buildroot/output/$RK_RECOVERY_CFG
	rm -rf "$RK_OUTDIR/recovery"
}

BUILD_CMDS="recovery"
build_hook()
{
	# No need to build recovery
	return 0
	[ -z "$RK_AB_UPDATE" ] || return 0

	check_config RK_RECOVERY_CFG || return 0

	echo "=========================================="
	echo "          Start building recovery(buildroot)"
	echo "=========================================="


	DST_DIR="$RK_OUTDIR/recovery"

	/usr/bin/time -f "you take %E to build recovery(buildroot)" \
		"$SCRIPTS_DIR/mk-buildroot.sh" $RK_RECOVERY_CFG "$DST_DIR"

	/usr/bin/time -f "you take %E to pack recovery image" \
		"$SCRIPTS_DIR/mk-ramdisk.sh" "$DST_DIR/rootfs.cpio.gz" \
		"$DST_DIR/recovery.img" "$RK_RECOVERY_FIT_ITS"
	ln -rsf "$DST_DIR/recovery.img" "$RK_FIRMWARE_DIR"

	# For security
	cp "$RK_FIRMWARE_DIR/recovery.img" u-boot/

	finish_build build_recovery
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

build_hook $@
