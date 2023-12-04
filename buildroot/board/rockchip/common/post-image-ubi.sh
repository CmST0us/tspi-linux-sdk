#!/bin/bash

BUILDROOT=$(pwd)
BUILDROOT_IMAGE_DIR=$1
BUILDROOT_HOST_DIR=$BUILDROOT_IMAGE_DIR/../host
BUILDROOT_DEFCONFIG=$BUILDROOT_IMAGE_DIR/../.config

ROOTFS_IMAGE_FAKEROOT_UBI=$BUILDROOT_IMAGE_DIR/fakeroot-ubi.fs
ROOTFS_SRC_DIR=$BUILDROOT_IMAGE_DIR/../target

IMAGE_OUTPUT_DIR=$BUILDROOT/../rockdev/
TOP_DIR=$BUILDROOT/../
FAKEROOT_TOOL=$BUILDROOT_HOST_DIR/bin/fakeroot
MKUBIFS_TOOL=$BUILDROOT_HOST_DIR/sbin/mkfs.ubifs
MKUBINIZE_TOOL=$BUILDROOT_HOST_DIR/sbin/ubinize
MKSQUASHFS_TOOL=$BUILDROOT_HOST_DIR/bin/mksquashfs

function unset_board_config_all()
{
	local tmp_file=`mktemp`
	grep -o "^export.*RK_.*=" `find $TOP_DIR/device/rockchip -name "Board*.mk" -type f` -h | sort | uniq > $tmp_file
	source $tmp_file
	rm -f $tmp_file
}

mk_ubi_image_fake()
{
	temp_dir="$BUILDROOT_IMAGE_DIR"

	if [ $(( $UBI_BLOCK_SIZE )) -eq $(( 0x20000 )) ]; then
		ubi_block_size_str="128KB"
	elif [ $(( $UBI_BLOCK_SIZE )) -eq $(( 0x40000 )) ]; then
		ubi_block_size_str="256KB"
	else
		echo "[$0:error] Please add ubi block size [$UBI_BLOCK_SIZE] to $PWD/$0"
		exit 1
	fi

	if [ $(( $UBI_PAGE_SIZE )) -eq 2048 ]; then
		ubi_page_size_str="2KB"
	elif [ $(( $UBI_PAGE_SIZE )) -eq 4096 ]; then
		ubi_page_size_str="4KB"
	else
		echo "[$0:error] Please add ubi block size [$UBI_PAGE_SIZE] to $PWD/$0"
		exit 1
	fi

	if [ -z "$UBI_VOL_NAME" ]; then
		echo "[$0:error] Please config ubifs partition volume name"
		exit 1
	fi

	ubifs_lebsize=$(( $UBI_BLOCK_SIZE - 2 * $UBI_PAGE_SIZE ))
	ubifs_miniosize=$UBI_PAGE_SIZE
	partition_size=$(( $UBI_PART_SIZE ))
	partition_size_str="$(( $partition_size / 1024 / 1024 ))MB"
	output_image=${IMAGE_OUTPUT_DIR}/${UBI_VOL_NAME}_${ubi_page_size_str}_${ubi_block_size_str}_${partition_size_str}.ubi
	temp_ubinize_file=$temp_dir/${UBI_VOL_NAME}_${ubi_page_size_str}_${ubi_block_size_str}_${partition_size_str}_ubinize.cfg
	UBI_VOL_TYPE=${UBI_VOL_TYPE:-dynamic}
	UBI_COMPRESSION_TPYE=${UBI_COMPRESSION_TPYE:-lzo}

	if [ $partition_size -le 0 ]; then
		echo "[$0:error] ubifs partition MUST set partition size"
		exit 1
	fi

	if [ ! -f $UBI_IMAGE_FAKEROOT ]; then
		echo "[$0:error] ubifs $UBI_IMAGE_FAKEROOT not found!!!"
		exit 1
	fi

	ubifs_maxlebcnt=$(( $partition_size / $ubifs_lebsize ))

	echo "[$0:info] ubifs_lebsize=$UBI_BLOCK_SIZE"
	echo "[$0:info] ubifs_miniosize=$UBI_PAGE_SIZE"
	echo "[$0:info] ubifs_maxlebcnt=$ubifs_maxlebcnt"

	case $UBIFS_TYPE in
		squashfs)
			temp_image=$temp_dir/${UBI_VOL_NAME}_${partition_size_str}.squashfs
			parallel_jobs=$((1 + `getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1`))

			if grep -w "BR2_TARGET_ROOTFS_SQUASHFS4_LZ4=y" $BUILDROOT_DEFCONFIG; then
				squashfs_compression_args=lz4
			elif grep -w "BR2_TARGET_ROOTFS_SQUASHFS4_LZO=y" $BUILDROOT_DEFCONFIG; then
				squashfs_compression_args=lzo
			elif grep -w "BR2_TARGET_ROOTFS_SQUASHFS4_LZMA=y" $BUILDROOT_DEFCONFIG; then
				squashfs_compression_args=lzma
			elif grep -w "BR2_TARGET_ROOTFS_SQUASHFS4_XZ=y" $BUILDROOT_DEFCONFIG; then
				squashfs_compression_args=xz
			elif grep -w "BR2_TARGET_ROOTFS_SQUASHFS4_ZSTD=y" $BUILDROOT_DEFCONFIG; then
				squashfs_compression_args=zstd
			else
				squashfs_compression_args=gzip
			fi

			if [ "$squashfs_compression_args" = "lz4" ]; then
				echo "$MKSQUASHFS_TOOL $UBI_SRC_DIR $temp_image -noappend -processors $parallel_jobs -comp $squashfs_compression_args -Xhc" >> $UBI_IMAGE_FAKEROOT
			else
				echo "$MKSQUASHFS_TOOL $UBI_SRC_DIR $temp_image -noappend -processors $parallel_jobs -comp $squashfs_compression_args" >> $UBI_IMAGE_FAKEROOT
			fi
			;;

		ubifs|*)
			temp_image=$temp_dir/${UBI_VOL_NAME}_${ubi_page_size_str}_${ubi_block_size_str}_${partition_size_str}.ubifs
			echo "$MKUBIFS_TOOL -x $UBI_COMPRESSION_TPYE -e $ubifs_lebsize -m $ubifs_miniosize -c $ubifs_maxlebcnt -d $UBI_SRC_DIR -F -v -o $temp_image" >> $UBI_IMAGE_FAKEROOT
			;;
	esac

	echo "[ubifs]" > $temp_ubinize_file
	echo "mode=ubi" >> $temp_ubinize_file
	echo "vol_id=0" >> $temp_ubinize_file
	echo "vol_type=$UBI_VOL_TYPE" >> $temp_ubinize_file
	echo "vol_name=$UBI_VOL_NAME" >> $temp_ubinize_file
	echo "vol_alignment=1" >> $temp_ubinize_file
	echo "vol_flags=autoresize" >> $temp_ubinize_file
	echo "image=$temp_image" >> $temp_ubinize_file
	echo "$MKUBINIZE_TOOL -o $output_image -m $ubifs_miniosize -p $UBI_BLOCK_SIZE -v $temp_ubinize_file" >> $UBI_IMAGE_FAKEROOT
	echo ""

	# Pick a default ubi image
	if [ $(( $DEFAULT_UBI_PAGE_SIZE )) -eq $(( $UBI_PAGE_SIZE )) \
		-a $(( $DEFAULT_UBI_BLOCK_SIZE )) -eq $(( $UBI_BLOCK_SIZE )) ]; then
		if [ "$UBI_VOL_NAME" = "rootfs" ]; then
			echo "rm -f $BUILDROOT_IMAGE_DIR/rootfs.ubi" >> $UBI_IMAGE_FAKEROOT
			echo "ln -rfs $output_image $BUILDROOT_IMAGE_DIR/rootfs.ubi" >> $UBI_IMAGE_FAKEROOT
		fi
	fi

	UBIFS_TYPE=
	UBI_BLOCK_SIZE=
	UBI_IMAGE_FAKEROOT=
	UBI_PAGE_SIZE=
	UBI_PART_SIZE=
	UBI_SRC_DIR=
	UBI_VOL_NAME=
	UBI_VOL_TYPE=
}

get_partition_size()
{
	echo $PARAMETER

	PARTITIONS_PREFIX=`echo -n "CMDLINE: mtdparts=rk29xxnand:"`
	while read line
	do
		if [[ $line =~ $PARTITIONS_PREFIX ]]
		then
			partitions=`echo $line | sed "s/$PARTITIONS_PREFIX//g"`
			echo $partitions
			break
		fi
	done < $PARAMETER

	[ -z $"partitions" ] && return

	IFS=,
	for part in $partitions;
	do
		part_size=`echo $part | cut -d '@' -f1`
		part_name=`echo $part | cut -d '(' -f2|cut -d ')' -f1`

		[[ $part_size =~ "-" ]] && continue

		part_size=$(($part_size))
		part_size_bytes=$[$part_size*512]

		case $part_name in
			rootfs | system_[ab])
				ROOTFS_PART_SIZE=$part_size_bytes
				;;
			oem)
				OEM_PART_SIZE=$part_size_bytes
				;;
		esac
	done
}

mk_ubi_image_fake_for_rootfs()
{
	UBI_BLOCK_SIZE=$1
	UBI_PAGE_SIZE=$2

	UBI_VOL_NAME="rootfs"
	UBI_PART_SIZE=$ROOTFS_PART_SIZE
	UBI_IMAGE_FAKEROOT=$ROOTFS_IMAGE_FAKEROOT_UBI
	UBI_SRC_DIR=$ROOTFS_SRC_DIR

	UBIFS_TYPE=`grep -w BR2_PACKAGE_ROOTFS_UBI_CUSTOM_FILESYSTEM $BUILDROOT_DEFCONFIG`
	UBIFS_TYPE=${UBIFS_TYPE##*=}
	UBIFS_TYPE=${UBIFS_TYPE%*\"}
	UBIFS_TYPE=${UBIFS_TYPE#*\"}
	case $UBIFS_TYPE in
		squashfs)
			UBI_VOL_TYPE=static
			;;
		ubifs|*)
			UBI_VOL_TYPE=dynamic
			;;
	esac

	mk_ubi_image_fake
}

get_ubi_image_compression_tpye()
{
	UBI_COMPRESSION_TPYE=""
	if grep -w "BR2_TARGET_ROOTFS_UBIFS_RT_ZLIB=y" $BUILDROOT_DEFCONFIG; then
		UBI_COMPRESSION_TPYE=zlib
	fi
	if grep -w "BR2_TARGET_ROOTFS_UBIFS_RT_LZO=y" $BUILDROOT_DEFCONFIG; then
		UBI_COMPRESSION_TPYE=lzo
	fi
}

stash_unused_files()
{
	# stash $ROOTFS_SRC_DIR/THIS_IS_NOT_YOUR_ROOT_FILESYSTEM
	mv $ROOTFS_SRC_DIR/THIS_IS_NOT_YOUR_ROOT_FILESYSTEM $BUILDROOT_IMAGE_DIR/
}

pop_unused_files()
{
	mv $BUILDROOT_IMAGE_DIR/THIS_IS_NOT_YOUR_ROOT_FILESYSTEM $ROOTFS_SRC_DIR/
}

# Start

if [ ! -d $IMAGE_OUTPUT_DIR ]; then
	mkdir -p $IMAGE_OUTPUT_DIR
fi

unset_board_config_all

# Get the parameter file
source $TOP_DIR/device/rockchip/.BoardConfig.mk
PARAMETER=$TOP_DIR/device/rockchip/.target_product/$RK_PARAMETER
# default page size 2KB
DEFAULT_UBI_PAGE_SIZE=${RK_UBI_PAGE_SIZE:-2048}
# default block size 128KB
DEFAULT_UBI_BLOCK_SIZE=${RK_UBI_BLOCK_SIZE:-0x20000}

stash_unused_files

echo "[$0:info] Start build ubi images..."

get_partition_size
get_ubi_image_compression_tpye

if which fakeroot; then
FAKEROOT_TOOL="`which fakeroot`"
else
	echo -e "[$0:error] Install fakeroot First."
	echo -e "[$0:error]   sudo apt-get install fakeroot"
	exit -1
fi

sed -i '/\<ubinize\>/d' $ROOTFS_IMAGE_FAKEROOT_UBI
mk_ubi_image_fake_for_rootfs 0x20000 2048
mk_ubi_image_fake_for_rootfs 0x40000 2048
mk_ubi_image_fake_for_rootfs 0x40000 4096
$FAKEROOT_TOOL -- $ROOTFS_IMAGE_FAKEROOT_UBI

echo "[$0:info] End build ubi images..."
pop_unused_files

exit 0
