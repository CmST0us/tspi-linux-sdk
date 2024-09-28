#!/bin/bash -e

RK_PACK_TOOL_DIR="$SDK_DIR/tools/linux/Linux_Pack_Firmware/rockdev"

gen_package_file()
{
	TYPE="${1:-update}"

	if [ ! -r parameter.txt ]; then
		echo -e "\e[31mparameter.txt is missing\e[0m" >&2
		exit 1
	fi

	echo -e "# NAME\tPATH"
	echo -e "package-file\tpackage-file"
	echo -e "parameter\tparameter.txt"

	[ ! -r MiniLoaderAll.bin ] || echo -e "bootloader\tMiniLoaderAll.bin"

	for part in $(rk_partition_parse_names parameter.txt); do
		if echo $part | grep -q "_b$"; then
			# Not packing *_b partition for ota|sdcard
			case $TYPE in
				ota|sdcard) continue ;;
			esac
		fi

		case $part in
			backup) echo -e "backup\tRESERVED"; continue ;;
			system|system_[ab]) IMAGE=rootfs.img ;;
			*_a) IMAGE="${part%_a}.img" ;;
			*_b) IMAGE="${part%_b}.img" ;;
			*) IMAGE="$part.img" ;;
		esac

		[ ! -r "$IMAGE" ] || echo -e "$part\t$IMAGE"
	done
}

build_updateimg()
{
	TARGET="${1:-$RK_FIRMWARE_DIR/update.img}"
	TYPE="${2:-update}"
	PKG_FILE="${3:-$RK_PACKAGE_FILE}"
	OUT_DIR="$RK_OUTDIR/$TYPE"
	IMAGE_DIR="$OUT_DIR/Image"

	echo "=========================================="
	echo "          Start packing $2 update image"
	echo "=========================================="

	rm -rf "$TARGET" "$OUT_DIR"
	mkdir -p "$IMAGE_DIR"
	cd "$IMAGE_DIR"

	# Prepare images
	ln -rsf "$RK_FIRMWARE_DIR"/* .
	if [ "$TYPE" = sdcard ]; then
		ln -rsf "$RK_IMAGE_DIR/sdupdate-ab-misc.img" misc.img
		ln -rsf "$RK_IMAGE_DIR/parameter-sdupdate.txt" parameter.txt

		# Not packing rootfs partition for sdcard
		rm -f rootfs.img
	fi

	# Prepare package-file
	if [ "$PKG_FILE" ]; then
		PKG_FILE="$CHIP_DIR/$PKG_FILE"
		if [ ! -r "$PKG_FILE" ]; then
			echo "$PKG_FILE not exists!"
			exit 1
		fi
		ln -rsf "$PKG_FILE" package-file
	else
		echo "Generating package-file for $TYPE :"
		gen_package_file $TYPE > package-file
		cat package-file
	fi

	echo "Packing $TARGET for $TYPE..."

	if [ ! -r MiniLoaderAll.bin ]; then
		echo -e "\e[31mMiniLoaderAll.bin is missing\e[0m"
		exit 1
	fi

	TAG=RK$(hexdump -s 21 -n 4 -e '4 "%c"' MiniLoaderAll.bin | rev)
	"$RK_PACK_TOOL_DIR/afptool" -pack ./ update.raw.img
	"$RK_PACK_TOOL_DIR/rkImageMaker" -$TAG MiniLoaderAll.bin \
		update.raw.img update.img -os_type:androidos

	ln -rsf "$IMAGE_DIR/package-file" "$OUT_DIR"
	ln -rsf "$IMAGE_DIR/update.img" "$OUT_DIR"
	ln -rsf "$IMAGE_DIR/update.img" "$TARGET"

	# rpdzkj add updata.img
	DTS_DIR="${SDK_DIR}/kernel/arch/arm64/boot/dts/rockchip/${RK_KERNEL_DTS_NAME}.dts"
	LCD_NAME=`grep '^#include.*rp.*lcd-[^g][^p]' $DTS_DIR | grep -Ev "hdmi|typec" |  sed 's/^#include.*-lcd-//g' | sed 's/.dts.*//g'`
	DATE=`date +%Y%m%d-%H%M%S`
	echo "${RK_FIRMWARE_DIR}/update-${RK_KERNEL_DTS_NAME/\//-}-$RK_ROOTFS_SYSTEM-$LCD_NAME-$DATE.img"
	rm -rf "${RK_FIRMWARE_DIR}"/update-*.img
	ln -rsf "$IMAGE_DIR/update.img" "${RK_FIRMWARE_DIR}/update-${RK_KERNEL_DTS_NAME/\//-}-$RK_ROOTFS_SYSTEM-$LCD_NAME-$DATE.img"

	finish_build build_updateimg $@
}

build_ota_updateimg()
{
	check_config RK_AB_UPDATE || return 0

	echo "Make A/B update image for OTA"

	build_updateimg "$RK_FIRMWARE_DIR/update_ota.img" ota \
		$RK_OTA_PACKAGE_FILE

	finish_build
}

build_sdcard_updateimg()
{
	check_config RK_AB_UPDATE RK_AB_UPDATE_SDCARD || return 0

	echo "Make A/B update image for SDcard"

	build_updateimg "$RK_FIRMWARE_DIR/update_sdcard.img" sdcard

	finish_build
}

build_ab_updateimg()
{
	check_config RK_AB_UPDATE || return 0

	build_ota_updateimg
	build_sdcard_updateimg

	echo "Make A/B update image"

	build_updateimg "$RK_FIRMWARE_DIR/update_ab.img" ab

	finish_build
}

# Hooks

usage_hook()
{
	echo -e "updateimg                         \tbuild update image"
	echo -e "otapackage                        \tbuild OTA update image"
	echo -e "sdpackage                         \tbuild SDcard update image"
}

clean_hook()
{
	rm -rf "$RK_OUTDIR/update"
	rm -rf "$RK_OUTDIR/ota"
	rm -rf "$RK_OUTDIR/sdcard"
	rm -rf "$RK_OUTDIR/ab"
	rm -rf "$RK_FIRMWARE_DIR/*update.img"
}

POST_BUILD_CMDS="updateimg otapackage sdpackage"
post_build_hook()
{
	case "$1" in
		updateimg)
			if [ "$RK_AB_UPDATE" ]; then
				build_ab_updateimg
			else
				build_updateimg
			fi
			;;
		otapackage) build_ota_updateimg ;;
		sdpackage) build_sdcard_updateimg ;;
		*) usage ;;
	esac
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

post_build_hook ${@:-updateimg}
