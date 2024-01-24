#!/bin/bash -e

build_sha256_boot()
{
	echo "Packing ramdisk with sha256"

	DIGEST=$(dirname "$RAMDISK_IMG")/ramdisk.gz.digest

	openssl dgst -sha256 -binary -out "$DIGEST" "$RAMDISK_IMG"
	DIGEST_SIZE=$(stat -c "%s" "$RAMDISK_IMG")

	gzip -fk "$RK_KERNEL_DTS"

cat << EOF >> "$RK_KERNEL_DTS"
&ramdisk_c {
	size = <$DIGEST_SIZE>;
	hash {
		algo = "sha256";
		value = /incbin/("$DIGEST");
	};
};
EOF
	"$SCRIPTS_DIR/mk-kernel.sh"

	gunzip -fk "$RK_KERNEL_DTS.gz"
}

RAMDISK_IMG="$1"
TARGET_IMG="$2"
ITS="$3"

if [ ! -f "$RAMDISK_IMG" ]; then
	echo "$RAMDISK_IMG doesn't exist"
	exit 0
fi

KERNEL_IMG="$RK_KERNEL_IMG"

if [ ! -f "$KERNEL_IMG" ]; then
	echo "Build kernel for initrd"
	"$SCRIPTS_DIR/mk-kernel.sh"
fi

if [ -n "$RK_ROOTFS_INITRD_COMPRESS" ]; then
	cat "$RAMDISK_IMG" | gzip -n -f -9 > "$RAMDISK_IMG.gz"
	cat "$KERNEL_IMG" | gzip -n -f -9 > "$KERNEL_IMG.gz"
	RAMDISK_IMG="$RAMDISK_IMG.gz"
	KERNEL_IMG="$KERNEL_IMG.gz"
fi

echo "Packing $RAMDISK_IMG to $TARGET_IMG"
if [ -n "$ITS" ]; then
	if [ -n "$RK_SECURITY" -a -z "$RK_SECURITY_CHECK_METHOD" ]; then
		build_sha256_boot
	fi

	"$SCRIPTS_DIR/mk-fitimage.sh" "$TARGET_IMG" "$ITS" \
		"$KERNEL_IMG" "$RAMDISK_IMG"
else
	kernel/scripts/mkbootimg --kernel "$KERNEL_IMG" \
		--ramdisk "$RAMDISK_IMG" --second "kernel/resource.img" \
		-o "$TARGET_IMG"
fi
