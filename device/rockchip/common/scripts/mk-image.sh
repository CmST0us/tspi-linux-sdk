#!/bin/bash

# Prefer using buildroot host tools for compatible.
if [ -n "$RK_BUILDROOT_CFG" ]; then
    HOST_DIR="$SDK_DIR/buildroot/output/$RK_BUILDROOT_CFG/host"
    export PATH=$HOST_DIR/usr/sbin:$HOST_DIR/usr/bin:$HOST_DIR/sbin:$HOST_DIR/bin:$PATH
    echo "Using host tools in $HOST_DIR"
else
    "$(dirname "$(realpath "$0")")/check-mkimage.sh"
fi

fatal()
{
    echo -e "FATAL: " $@
    exit 1
}

usage()
{
    echo $@
    fatal "Usage: $0 <src_dir> <target_image> <fs_type> <size(M|K)|auto(0)> [label]"
}

[ ! $# -lt 4 ] || usage "Not enough args${@+: $0 $@}"

export SRC_DIR=$1
export TARGET=$2
FS_TYPE=$3
SIZE=$4
LABEL=$5

case $SIZE in
    auto)
        SIZE_KB=0
        ;;
    *K)
        SIZE_KB=$(( ${SIZE%K} ))
        ;;
    *)
        SIZE_KB=$(( ${SIZE%M} * 1024 )) # default is MB
        ;;
esac

echo $SIZE_KB | grep -vq [^0-9] || usage "Invalid size: $SIZE_KB"

TEMP=$(mktemp -u)

[ -d "$SRC_DIR" ] || usage "No such src dir: $SRC_DIR"

copy_to_ntfs()
{
    DEPTH=1
    while true;do
        find $SRC_DIR -maxdepth $DEPTH -mindepth $DEPTH -type d|grep -q "" \
            || break
        find $SRC_DIR -maxdepth $DEPTH -mindepth $DEPTH -type d \
            -exec sh -c 'ntfscp $TARGET "$1" "${1#$SRC_DIR}"' sh {} \; || \
	    fatal "Detected non-buildroot ntfscp(doesn't support dir copy)"
        DEPTH=$(($DEPTH + 1))
    done

    find $SRC_DIR -type f \
        -exec sh -c 'ntfscp $TARGET "$1" "${1#$SRC_DIR}"' sh {} \; || \
            fatal "Failed to do ntfscp!"
}

copy_to_image()
{
    ls $SRC_DIR/* &>/dev/null || return 0

    echo "Copying $SRC_DIR into $TARGET (root permission required)"
    mkdir -p $TEMP || return 1
    sudo mount $TARGET $TEMP || return 1

    cp -rp $SRC_DIR/* $TEMP
    RET=$?

    sudo umount $TEMP
    rm -rf $TEMP

    return $RET
}

check_host_tool()
{
    which $1|grep -wq buildroot
}

mkimage()
{
    echo "Making $TARGET from $SRC_DIR with size(${SIZE_KB}KB)"
    rm -rf $TARGET
    dd of=$TARGET bs=1K seek=$SIZE_KB count=0 &>/dev/null || \
        fatal "Failed to dd image!"
    case $FS_TYPE in
        ext[234])
            if mke2fs -h 2>&1 | grep -wq "\-d"; then
                mke2fs -t $FS_TYPE $TARGET -d $SRC_DIR \
                    || return 1
            else
                echo "Detected old mke2fs(doesn't support '-d' option)!"
                mke2fs -t $FS_TYPE $TARGET || return 1
                copy_to_image || return 1
            fi
            # Set max-mount-counts to 0, and disable the time-dependent checking.
            tune2fs -c 0 -i 0 $TARGET ${LABEL:+-L $LABEL}
            ;;
        msdos|fat|vfat)
            # Use fat32 by default
            mkfs.vfat -F 32 $TARGET ${LABEL:+-n $LABEL} && \
                MTOOLS_SKIP_CHECK=1 \
                mcopy -bspmn -D s -i $TARGET $SRC_DIR/* ::/
            ;;
        ntfs)
            # Enable compression
            mkntfs -FCQ $TARGET ${LABEL:+-L $LABEL}
            if check_host_tool ntfscp; then
                copy_to_ntfs
            else
                copy_to_image
            fi
            ;;
        ubi|ubifs)
            mk_ubi_image
            ;;
    esac
}

mkimage_auto_sized()
{
    tar cf $TEMP $SRC_DIR &>/dev/null
    SIZE_KB=$(du -k $TEMP|grep -o "^[0-9]*")
    rm -rf $TEMP
    echo "Making $TARGET from $SRC_DIR (auto sized)"

    MAX_RETRY=10
    RETRY=0

    while true;do
        EXTRA_SIZE=$(($SIZE_KB / 50))
        SIZE_KB=$(($SIZE_KB + ($EXTRA_SIZE > 4096 ? $EXTRA_SIZE : 4096)))
        mkimage && break

        RETRY=$[RETRY+1]
        [ $RETRY -gt $MAX_RETRY ] && fatal "Failed to make image!"
        echo "Retring with increased size....($RETRY/$MAX_RETRY)"
    done
}

mk_ubi_image()
{
    TARGET_DIR="${RK_OUTDIR:-$(dirname "$TARGET")}"
    UBI_VOL_NAME=${LABEL:-ubi}

    # default page size 2KB
    UBI_PAGE_SIZE=${RK_UBI_PAGE_SIZE:-2048}
    # default block size 128KB
    UBI_BLOCK_SIZE=${RK_UBI_BLOCK_SIZE:-0x20000}

    UBIFS_LEBSIZE=$(( $UBI_BLOCK_SIZE - 2 * $UBI_PAGE_SIZE ))
    UBIFS_MINIOSIZE=$UBI_PAGE_SIZE

    UBIFS_IMAGE="$TARGET_DIR/$UBI_VOL_NAME.ubifs"
    UBINIZE_CFG="$TARGET_DIR/${UBI_VOL_NAME}-ubinize.cfg"

    UBIFS_MAXLEBCNT=$(( $SIZE_KB * 1024 / $UBIFS_LEBSIZE ))

    mkfs.ubifs -x lzo -e $UBIFS_LEBSIZE -m $UBIFS_MINIOSIZE \
        -c $UBIFS_MAXLEBCNT -d $SRC_DIR -F -v -o $UBIFS_IMAGE || return 1

    echo "[ubifs]" > $UBINIZE_CFG
    echo "mode=ubi" >> $UBINIZE_CFG
    echo "vol_id=0" >> $UBINIZE_CFG
    echo "vol_type=dynamic" >> $UBINIZE_CFG
    echo "vol_name=$UBI_VOL_NAME" >> $UBINIZE_CFG
    echo "vol_size=${SIZE_KB}KiB" >> $UBINIZE_CFG
    echo "vol_alignment=1" >> $UBINIZE_CFG
    echo "vol_flags=autoresize" >> $UBINIZE_CFG
    echo "image=$UBIFS_IMAGE" >> $UBINIZE_CFG
    ubinize -o $TARGET -m $UBIFS_MINIOSIZE -p $UBI_BLOCK_SIZE \
        -v $UBINIZE_CFG
}

rm -rf $TARGET
case $FS_TYPE in
    ext[234]|msdos|fat|vfat|ntfs|ubi|ubifs)
        if [ $SIZE_KB -eq 0 ]; then
            mkimage_auto_sized
        else
            mkimage && echo "Generated $TARGET"
        fi
        ;;
    squashfs)
        [ $SIZE_KB -eq 0 ] || fatal "$FS_TYPE: fixed size not supported."
        mksquashfs $SRC_DIR $TARGET -noappend -comp lz4
        ;;
    jffs2)
        [ $SIZE_KB -eq 0 ] || fatal "$FS_TYPE: fixed size not supported."
        mkfs.jffs2 -r $SRC_DIR -o $TARGET 0x10000 --pad=0x400000 -s 0x1000 -n
        ;;
    *)
        usage "File system: $FS_TYPE not supported."
        ;;
esac
