#!/bin/bash

set -e

if ! which fakeroot; then
    echo "fakeroot not found! (sudo apt-get install fakeroot)"
    exit -1
fi

SCRIPT_DIR=$(dirname $(realpath $BASH_SOURCE))
TOP_DIR=$(realpath $SCRIPT_DIR/../../..)
cd $TOP_DIR

DEV_DIR="$TOP_DIR/device/rockchip"
OUT_DIR="$TOP_DIR/buildroot/output"
IMG_DIR="$OUT_DIR/$RK_CFG_BUILDROOT/images"

function unset_board_config_all()
{
    local tmp_file=`mktemp`
    grep -o "^export.*RK_.*=" `find $DEV_DIR -name "Board*.mk" -type f` -h \
        | sort | uniq > $tmp_file
    source $tmp_file
    rm -f $tmp_file
}
unset_board_config_all

source $DEV_DIR/.BoardConfig.mk
ROCKDEV=$TOP_DIR/rockdev
PARAMETER=$DEV_DIR/$RK_TARGET_PRODUCT/$RK_PARAMETER
MISC_IMG=$DEV_DIR/rockimg/$RK_MISC
if [ "$RK_RAMDISK_SECURITY_BOOTUP" = "true" ];then
	ROOTFS_IMG=$IMG_DIR/security-system.img
else
	ROOTFS_IMG=$TOP_DIR/$RK_ROOTFS_IMG
fi

ROOTFS_IMG_SOURCE=$IMG_DIR/rootfs.$RK_ROOTFS_TYPE
RAMBOOT_IMG=$OUT_DIR/$RK_CFG_RAMBOOT/images/ramboot.img
RECOVERY_IMG=$OUT_DIR/$RK_CFG_RECOVERY/images/recovery.img
TRUST_IMG=$TOP_DIR/u-boot/trust.img
UBOOT_IMG=$TOP_DIR/u-boot/uboot.img
BOOT_IMG=$TOP_DIR/kernel/$RK_BOOT_IMG
LOADER=$(echo $TOP_DIR/u-boot/*_loader_*v*.bin | head -1)
SPL=$(echo $TOP_DIR/u-boot/*_loader_spl.bin | head -1)
MKIMAGE=$SCRIPT_DIR/mk-image.sh

message() {
    echo -e "\e[36m $@ \e[0m"
}

fatal() {
    echo -e "\e[31m $@ \e[0m"
    exit -1
}

mkdir -p $ROCKDEV

# Require buildroot host tools to do image packing.
if [ ! -d "$TARGET_OUTPUT_DIR" ]; then
    message "Source buildroot/build/envsetup.sh"
    if [ "${RK_CFG_RAMBOOT}" ];then
        source $TOP_DIR/buildroot/build/envsetup.sh $RK_CFG_RAMBOOT
    fi
    if [ "${RK_CFG_BUILDROOT}" ];then
        source $TOP_DIR/buildroot/build/envsetup.sh $RK_CFG_BUILDROOT
    fi
fi

# Parse size limit from parameter.txt, 0 means unlimited or not exists.
partition_size_kb() {
    PART_NAME=$1
    PART_STR=$(grep -oE "[^,^:^\(]*\(${PART_NAME}[\)_:][^\)]*\)" $PARAMETER)
    PART_SIZE=$(echo $PART_STR | grep -oE "^[^@^-]*")
    echo $(( ${PART_SIZE:-0} / 2 ))
}

# Assert the image's size smaller than parameter.txt's limit
assert_size() {
    PART_NAME="$1"
    IMG="$2"

    PART_SIZE=$(partition_size_kb $PART_NAME)
    [ "$PART_SIZE" -gt 0 ] || return 0

    IMG_SIZE=$(stat -c "%s" "$IMG")

    if [ $PART_SIZE -lt $(( "$IMG_SIZE" / 1024 )) ]; then
        fatal "error: $IMG's size exceed parameter.txt's limit!"
    fi
}

link_image() {
    SRC="$1"
    DST="$2"
    FALLBACK="$3"

    message "Linking $DST from $SRC..."

    if [ ! -e "$SRC" ]; then
        if [ -e "$FALLBACK" ]; then
            SRC="$FALLBACK"
            message "Fallback to $SRC"
        else
            message "warning: $SRC not found!"
        fi
    fi

    ln -rsf "$SRC" "$ROCKDEV/$DST"
    assert_size "${DST%.img}" "$SRC"

    message "Done linking $DST"
}

link_image_optional() {
    link_image "$@" || true
}

pack_image() {
    SRC="$1"
    DST="$2"
    FS_TYPE="$3"
    SIZE="${4:-$(partition_size_kb "${DST%.img}")}"
    LABEL="$5"
    EXTRA_CMD="$6"

    FAKEROOT_SCRIPT="$ROCKDEV/${DST%.img}.fs"

    message "Packing $DST from $SRC..."

    if [ ! -d "$SRC" ]; then
        message "warning: $SRC not found!"
        return 0
    fi

    cat << EOF > $FAKEROOT_SCRIPT
#!/bin/sh -e
$EXTRA_CMD
$MKIMAGE "$SRC" "$ROCKDEV/$DST" "$FS_TYPE" "$SIZE" "$LABEL"
EOF

    chmod a+x "$FAKEROOT_SCRIPT"
    fakeroot -- "$FAKEROOT_SCRIPT"
    rm -f "$FAKEROOT_SCRIPT"

    assert_size "${DST%.img}" "$ROCKDEV/$DST"

    message "Done packing $DST"
}

# Convert legacy partition variables to new style
legacy_partion() {
    PART_NAME="$1"
    SRC="$2"
    FS_TYPE="$3"
    SIZE="${4:-0}"
    MOUNT="/$PART_NAME"
    OPT=""

    [ "$FS_TYPE" ] || return 0
    [ "$SRC" ] || return 0

    # Fixed size for ubi
    if [ "$FS_TYPE" = ubi ]; then
        OPT="fixed"
    fi

    case $SIZE in
        *k|*K)
            SIZE=${SIZE//k/K}
            ;;
        *m|*M)
            SIZE=${SIZE//m/M}
            ;;
        *)
            SIZE=$(( ${SIZE} / 1024 ))K # default is bytes
            ;;
    esac

    echo "$PART_NAME:$MOUNT:$FS_TYPE:defaults:$SRC:${SIZE}:$OPT"
}

RK_LEGACY_PARTITIONS=" \
    $(legacy_partion oem "$RK_OEM_DIR" "$RK_OEM_FS_TYPE" "$RK_OEM_PARTITION_SIZE")
    $(legacy_partion userdata "$RK_USERDATA_DIR" "$RK_USERDATA_FS_TYPE" "$RK_USERDATA_PARTITION_SIZE")
"

# <dev>:<mount point>:<fs type>:<mount flags>:<source dir>:<image size(M|K|auto)>:[options]
# for example:
# RK_EXTRA_PARTITIONS="oem:/oem:ext2:defaults:oem_normal:256M:fixed
# userdata:/userdata:vfat:errors=remount-ro:userdata_empty:auto"
RK_EXTRA_PARTITIONS="${RK_EXTRA_PARTITIONS:-${RK_LEGACY_PARTITIONS}}"

partition_arg() {
    PART="$1"
    I="$2"
    DEFAULT="$3"

    ARG=$(echo $PART | cut -d':' -f"$I")
    echo ${ARG:-$DEFAULT}
}

pack_extra_partitions() {
    for part in ${RK_EXTRA_PARTITIONS//@/ }; do
        DEV="$(partition_arg "$part" 1)"

        # Dev is either <name> or /dev/.../<name>
        [ "$DEV" ] || continue
        PART_NAME="${DEV##*/}"

        MOUNT="$(partition_arg "$part" 2 "/$PART_NAME")"
        FS_TYPE="$(partition_arg "$part" 3)"

        SRC="$(partition_arg "$part" 5)"

        # Src is either none or relative path to device/rockchip/<name>/
        # or absolute path
        case "$SRC" in
            "")
                continue
                ;;
            /*)
                ;;
            *)
                SRC="$DEV_DIR/$PART_NAME/$SRC"
                ;;
        esac

        SIZE="$(partition_arg "$part" 6 auto)"
        OPTS="$(partition_arg "$part" 7)"
        LABEL=
        EXTRA_CMD=

        # Special handling for oem
        if [ "$PART_NAME" = oem ]; then
            # Skip packing oem when builtin
            [ "${RK_OEM_BUILDIN_BUILDROOT}" != "YES" ] || continue

            if [ -d "$SRC/www" ]; then
                EXTRA_CMD="chown -R www-data:www-data $SRC/www"
            fi
        fi

        # Skip boot time resize by adding a label
        if echo $OPTS | grep -wq fixed; then
            LABEL="$PART_NAME"
        fi

        pack_image "$SRC" "${PART_NAME}.img" "$FS_TYPE" "$SIZE" "$LABEL" \
            "$EXTRA_CMD"
    done
}

link_image_optional "$PARAMETER" parameter.txt

link_image_optional "$UBOOT_IMG" uboot.img

[ "$RK_UBOOT_FORMAT_TYPE" != "fit" ] && \
    link_image_optional "$TRUST_IMG" trust.img

link_image_optional "$LOADER" MiniLoaderAll.bin "$SPL"

[ "$RK_BOOT_IMG" ] && link_image_optional "$BOOT_IMG" boot.img

[ $RK_CFG_RAMBOOT ] && link_image_optional "$RAMBOOT_IMG" boot.img

[ "$RK_CFG_RECOVERY" ] && link_image_optional "$RECOVERY_IMG" recovery.img

[ "$RK_MISC" ] && \
    link_image_optional "$DEV_DIR/rockimg/misc.img" misc.img "$MISC_IMG"

[ "$RK_ROOTFS_IMG" ] && \
    link_image_optional "$ROOTFS_IMG" rootfs.img "$ROOTFS_IMG_SOURCE"

[ "${RK_OEM_BUILDIN_BUILDROOT}" = "YES" ] && \
    link_image_optional "$IMG_DIR/oem.img" oem.img

if [ "$RK_RAMDISK_SECURITY_BOOTUP" = "true" ]; then
    for part in boot recovery rootfs;do
        test -e $TOP_DIR/u-boot/${part}.img &&
            link_image "$TOP_DIR/u-boot/${part}.img" ${part}.img && \
                message "Enabled ramdisk security $part..." || true
    done
fi

pack_extra_partitions

message "Images in $ROCKDEV are ready!"
