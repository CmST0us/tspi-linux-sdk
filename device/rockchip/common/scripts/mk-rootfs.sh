#!/bin/bash -e

build_buildroot()
{
	check_config RK_BUILDROOT_CFG || return 0

	ROOTFS_DIR="${1:-$RK_OUTDIR/buildroot}"

	/usr/bin/time -f "you take %E to build buildroot" \
		"$SCRIPTS_DIR/mk-buildroot.sh" $RK_BUILDROOT_CFG "$ROOTFS_DIR"

	cat "$RK_LOG_DIR/post-rootfs.log"
	finish_build build_buildroot $@
}

build_yocto()
{
	check_config RK_YOCTO_CFG RK_KERNEL_VERSION_REAL || return 0

	ROOTFS_DIR="${1:-$RK_OUTDIR/yocto}"

	"$SCRIPTS_DIR/check-yocto.sh"

	cd yocto
	ln -sf $RK_YOCTO_CFG.conf build/conf/local.conf

	{
		echo "PREFERRED_VERSION_linux-rockchip := \"$RK_KERNEL_VERSION_REAL%\""
		echo "LINUXLIBCVERSION := \"$RK_KERNEL_VERSION_REAL-custom%\""
		case "$RK_CHIP_FAMILY" in
			px30|rk3326|rk3562|rk3566_rk3568|rk3588)
				echo "MALI_VERSION := \"g13p0\"" ;;
		esac
		echo "DISPLAY_PLATFORM := \"$RK_YOCTO_DISPLAY_PLATFORM\""
	} > build/rksdk-override.conf

	source oe-init-build-env build
	LANG=en_US.UTF-8 LANGUAGE=en_US.en LC_ALL=en_US.UTF-8 \
		bitbake core-image-minimal -f -c rootfs -c image_complete \
		-R conf/include/rksdk.conf -R rksdk-override.conf

	ln -rsf "$PWD/latest/rootfs.img" $ROOTFS_DIR/rootfs.ext4

	touch "$RK_LOG_DIR/post-rootfs.log"
	cat "$RK_LOG_DIR/post-rootfs.log"
	finish_build build_yocto $@
}

build_debian()
{
	check_config RK_DEBIAN_VERSION || return 0

	ROOTFS_DIR="${1:-$RK_OUTDIR/debian}"
	ARCH=${RK_DEBIAN_ARCH:-armhf}

	"$SCRIPTS_DIR/check-debian.sh"

	cd debian
	if [ ! -f linaro-$RK_DEBIAN_VERSION-alip-*.tar.gz ]; then
		RELEASE=$RK_DEBIAN_VERSION TARGET=desktop ARCH=$ARCH \
			./mk-base-debian.sh
		ln -sf linaro-$RK_DEBIAN_VERSION-alip-*.tar.gz \
			linaro-$RK_DEBIAN_VERSION-$ARCH.tar.gz
	fi

	VERSION=debug ARCH=$ARCH ./mk-rootfs-$RK_DEBIAN_VERSION.sh
	./mk-image.sh

	ln -rsf "$PWD/linaro-rootfs.img" $ROOTFS_DIR/rootfs.ext4

	finish_build build_debian $@
}

function build_rootfs_distroboot(){
	cd ${SDK_DIR}
	echo -e "\e[36m Generate boot extlinux files\e[0m"

	KERNEL_VERSION=$(cat $SDK_DIR/kernel/include/config/kernel.release)
	ROOTFS_BOOT_DIR=${SDK_DIR}/${RK_ROOTFS_SYSTEM}/.boot
	KERNEL_DTS_FILENAME=${RK_KERNEL_DTS_NAME}

	if [ -d ${ROOTFS_BOOT_DIR} ]; then
		rm -rf ${ROOTFS_BOOT_DIR}
	fi

	mkdir -p ${ROOTFS_BOOT_DIR}/extlinux

	echo "label ${KERNEL_DTS_FILENAME} linux-${KERNEL_VERSION}" > ${ROOTFS_BOOT_DIR}/extlinux/extlinux.conf
	echo -e "\tkernel /boot/Image-${KERNEL_VERSION}" >> ${ROOTFS_BOOT_DIR}/extlinux/extlinux.conf
	echo -e "\tfdt /boot/${KERNEL_DTS_FILENAME}.dtb" >> ${ROOTFS_BOOT_DIR}/extlinux/extlinux.conf
	echo -e "\tappend ${RK_KERNEL_DISTROBOOT_CMD}" >> ${ROOTFS_BOOT_DIR}/extlinux/extlinux.conf

	cd ${SDK_DIR}
	cp ${RK_KERNEL_IMG} ${ROOTFS_BOOT_DIR}/Image-${KERNEL_VERSION}
    cp ${RK_KERNEL_DTB} ${ROOTFS_BOOT_DIR}
    ln -sf ${KERNEL_DTS_FILENAME}.dtb ${ROOTFS_BOOT_DIR}/rk-kernel.dtb
    cp kernel/.config ${ROOTFS_BOOT_DIR}/config-${KERNEL_VERSION}
    cp kernel/System.map ${ROOTFS_BOOT_DIR}/System.map-${KERNEL_VERSION}
    sudo cp -rf ${ROOTFS_BOOT_DIR}/* ${SDK_DIR}/${RK_ROOTFS_SYSTEM}/.${RK_ROOTFS_SYSTEM}-rootfs/boot
}

build_manjaro()
{
	# To build manjaro, first need to mount image, than install kernel module into manjaro
	if [ ! -f manjaro/manjaro.tar.gz ]; then
		echo -e "找不到Manjaro Rootfs Tar 包"
		return -1
	else

		echo "==============Start building manjaro =============="
		echo "TARGET_ARCH          =$RK_KERNEL_ARCH"
		echo "TARGET_KERNEL_CONFIG =$RK_KERNEL_CFG"
		echo "TARGET_KERNEL_CONFIG_FRAGMENT =$RK_KERNEL_CFG_FRAGMENTS"
		echo "=================================================="

		echo "Decompress manjaro rootfs"
		mkdir -p manjaro/.manjaro-rootfs
		sudo tar zxf manjaro/manjaro.tar.gz -C manjaro/.manjaro-rootfs

		if [ "${RK_KERNEL_DISTROBOOT}" = "y" ]; then
			echo "Build rootfs distroboot"
			build_rootfs_distroboot
		fi 

		cd ${SDK_DIR}
		echo "Build and install kernel module"
		mkdir -p ${SDK_DIR}/manjaro/.kernel_module_install
		$KMAKE INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=${SDK_DIR}/manjaro/.kernel_module_install modules_install

		sudo cp -ax manjaro/.kernel_module_install/lib/modules manjaro/.manjaro-rootfs/lib

		echo "Copy firmware"
		if [ ! -d manjaro/.manjaro-rootfs/vendor/etc/firmware ]; then
			sudo mkdir -p manjaro/.manjaro-rootfs/vendor/etc/firmware
		fi
		sudo cp -ax ${SDK_DIR}/firmware/* manjaro/.manjaro-rootfs/vendor/etc/firmware

		echo "Pack manjaro rootfs"
		cd ${SDK_DIR}
		dd if=/dev/zero of=manjaro/manjaro-rootfs.img bs=1G count=6
		mkfs.ext4 manjaro/manjaro-rootfs.img
		mkdir -p manjaro/.manjaro-rootfs-img
		sudo mount -o loop manjaro/manjaro-rootfs.img manjaro/.manjaro-rootfs-img
		sudo cp -ax manjaro/.manjaro-rootfs/* manjaro/.manjaro-rootfs-img/

		sudo umount manjaro/.manjaro-rootfs-img/
		e2fsck -p -f manjaro/manjaro-rootfs.img
		resize2fs -M manjaro/manjaro-rootfs.img

		echo "Finish build manjaro rootfs image, clean tmp file"
		sudo rm -rf manjaro/.manjaro* manjaro/.kernel*
	fi
}

build_ubuntu()
{
	# To build ubuntu, first need to mount image, than install kernel module into ubuntu
	if [ ! -f ubuntu/ubuntu.tar.gz ]; then
		echo -e "找不到Ubuntu Rootfs Tar 包"
		return -1
	else

		echo "==============Start building ubuntu =============="
		echo "TARGET_ARCH          =$RK_KERNEL_ARCH"
		echo "TARGET_KERNEL_CONFIG =$RK_KERNEL_CFG"
		echo "TARGET_KERNEL_CONFIG_FRAGMENT =$RK_KERNEL_CFG_FRAGMENTS"
		echo "=================================================="

		echo "Decompress ubuntu rootfs"
		mkdir -p ubuntu/.ubuntu-rootfs
		sudo tar zxf ubuntu/ubuntu.tar.gz -C ubuntu/.ubuntu-rootfs

		if [ "${RK_KERNEL_DISTROBOOT}" = "y" ]; then
			echo "Build rootfs distroboot"
			build_rootfs_distroboot
		fi 

		cd ${SDK_DIR}
		echo "Build and install kernel module"
		mkdir -p ${SDK_DIR}/ubuntu/.kernel_module_install
		$KMAKE INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=${SDK_DIR}/ubuntu/.kernel_module_install modules_install

		sudo cp -ax ubuntu/.kernel_module_install/lib/modules ubuntu/.ubuntu-rootfs/lib

		echo "Copy firmware"
		if [ ! -d ubuntu/.ubuntu-rootfs/vendor/etc/firmware ]; then
			sudo mkdir -p ubuntu/.ubuntu-rootfs/vendor/etc/firmware
		fi
		sudo cp -ax ${SDK_DIR}/firmware/* ubuntu/.ubuntu-rootfs/vendor/etc/firmware

		echo "Pack ubuntu rootfs"
		cd ${SDK_DIR}
		dd if=/dev/zero of=ubuntu/ubuntu-rootfs.img bs=1G count=4
		mkfs.ext4 ubuntu/ubuntu-rootfs.img
		mkdir -p ubuntu/.ubuntu-rootfs-img
		sudo mount -o loop ubuntu/ubuntu-rootfs.img ubuntu/.ubuntu-rootfs-img
		sudo cp -ax ubuntu/.ubuntu-rootfs/* ubuntu/.ubuntu-rootfs-img/

		sudo umount ubuntu/.ubuntu-rootfs-img/
		e2fsck -p -f ubuntu/ubuntu-rootfs.img
		resize2fs -M ubuntu/ubuntu-rootfs.img

		echo "Finish build ubuntu rootfs image, clean tmp file"
		sudo rm -rf ubuntu/.ubuntu* ubuntu/.kernel*
	fi
}

# Hooks

usage_hook()
{
	echo -e "buildroot-config[:<config>]       \tmodify buildroot defconfig"
	echo -e "rootfs[:<rootfs type>]            \tbuild default rootfs"
	echo -e "buildroot                         \tbuild buildroot rootfs"
	echo -e "yocto                             \tbuild yocto rootfs"
	echo -e "debian                            \tbuild debian rootfs"
}

clean_hook()
{
	rm -rf yocto/build/tmp yocto/build/*cache
	rm -rf debian/binary

	if check_config RK_BUILDROOT_CFG &>/dev/null; then
		rm -rf buildroot/output/$RK_BUILDROOT_CFG
	fi

	rm -rf "$RK_OUTDIR/buildroot"
	rm -rf "$RK_OUTDIR/yocto"
	rm -rf "$RK_OUTDIR/debian"
	rm -rf "$RK_OUTDIR/rootfs"
}

PRE_BUILD_CMDS="buildroot-config"
pre_build_hook()
{
	BUILDROOT_BOARD="${2:-"$RK_BUILDROOT_CFG"}"

	[ "$BUILDROOT_BOARD" ] || return 0

	TEMP_DIR=$(mktemp -d)
	"$SDK_DIR/buildroot/build/parse_defconfig.sh" "$BUILDROOT_BOARD" \
		"$TEMP_DIR/.config"
	make -C "$SDK_DIR/buildroot" O="$TEMP_DIR" menuconfig
	"$SDK_DIR/buildroot/build/update_defconfig.sh" "$BUILDROOT_BOARD" \
		"$TEMP_DIR"

	finish_build $@
}

BUILD_CMDS="rootfs buildroot debian yocto ubuntu"
build_hook()
{
	check_config RK_ROOTFS_TYPE || return 0

	if [ -z "$1" -o "$1" = rootfs ]; then
		ROOTFS=${RK_ROOTFS_SYSTEM:-buildroot}
	else
		ROOTFS=$1
	fi

	ROOTFS_IMG=rootfs.${RK_ROOTFS_TYPE}
	ROOTFS_DIR="$RK_OUTDIR/rootfs"

	echo "=========================================="
	echo "          Start building rootfs($ROOTFS)"
	echo "=========================================="

	rm -rf "$ROOTFS_DIR"
	mkdir -p "$ROOTFS_DIR"

	case "$ROOTFS" in
		yocto) build_yocto "$ROOTFS_DIR" ;;
		#debian) build_debian "$ROOTFS_DIR" ;;
		debian)
			if [ ! -f debian/linaro-rootfs.img ]; then
                                echo ""
                                echo -e "\033[31m找不到linaro-rootfs.img文件，请先将网盘链接中的debian镜像放到debian/文件夹下，并确保名称为linaro-rootfs.img\033[0m"
                                echo -e "\033[31mlinaro-rootfs.img file cannot be found.\033[0m"
                                echo -e "\033[31mPlease put the debian image from the network disk link under debian/ folder first, and make sure the name is linaro-rootfs.img.\033[0m"
                                echo ""
                        else
                                ln -srf debian/linaro-rootfs.img $ROOTFS_DIR/$ROOTFS_IMG
                        fi
                        ;;

		buildroot) build_buildroot "$ROOTFS_DIR" ;;
		ubuntu)
			build_ubuntu
			if [ ! -f ubuntu/ubuntu-rootfs.img ]; then
				echo ""
				echo -e "\033[31m找不到ubuntu.img文件，请先将网盘链接中的ubuntu镜像放到ubuntu/文件夹下，并确保名称为ubuntu.img\033[0m"
				echo -e "\033[31mubuntu.img file cannot be found.\033[0m"
				echo -e "\033[31mPlease put the ubuntu image from the network disk link under ubuntu/ folder first, and make sure the name is ubuntu.img.\033[0m"
				echo ""
			else
				ln -srf ubuntu/ubuntu-rootfs.img $ROOTFS_DIR/$ROOTFS_IMG
			fi
			;;

		manjaro)
			build_manjaro
			if [ ! -f manjaro/manjaro-rootfs.img ]; then
				echo ""
				echo -e "\033[31m找不到manjaro.img文件，请先将网盘链接中的manjaro镜像放到manjaro/文件夹下，并确保名称为ubuntu.img\033[0m"
				echo -e "\033[31mmanjaro.img file cannot be found.\033[0m"
				echo -e "\033[31mPlease put the manjaro image from the network disk link under manjaro/ folder first, and make sure the name is manjaro.img.\033[0m"
				echo ""
			else
				ln -srf manjaro/manjaro-rootfs.img $ROOTFS_DIR/$ROOTFS_IMG
			fi
			;;

		openkylin)
			if [ ! -f openkylin/openkylin.img ]; then
				echo ""
				echo -e "\033[31m找不到openkylin.img文件，请先将网盘链接中的openkylin镜像放到openkylin/文件夹下，并确保名称为openkylin.img\033[0m"
				echo -e "\033[31mopenkylin.img file cannot be found.\033[0m"
				echo -e "\033[31mPlease put the openkylin image from the network disk link under openkylin/ folder first, and make sure the name is openkylin.img.\033[0m"
				echo ""
			else
				ln -srf openkylin/openkylin.img $ROOTFS_DIR/$ROOTFS_IMG
			fi
			;;

		*) usage ;;
	esac

	if [ ! -f "$ROOTFS_DIR/$ROOTFS_IMG" ]; then
		echo "There's no $ROOTFS_IMG generated..."
		exit 1
	fi

	ln -rsf "$ROOTFS_DIR/$ROOTFS_IMG" "$RK_FIRMWARE_DIR/rootfs.img"

	# For builtin OEM image
	[ ! -e "$ROOTFS_DIR/oem.img" ] || \
		ln -rsf "$ROOTFS_DIR/oem.img" "$RK_FIRMWARE_DIR"

	if [ "$RK_ROOTFS_INITRD" ]; then
		/usr/bin/time -f "you take %E to pack ramboot image" \
			"$SCRIPTS_DIR/mk-ramdisk.sh" \
			"$RK_FIRMWARE_DIR/rootfs.img" \
			"$ROOTFS_DIR/ramboot.img" "$RK_BOOT_FIT_ITS"
		ln -rsf "$ROOTFS_DIR/ramboot.img" \
			"$RK_FIRMWARE_DIR/boot.img"

		# For security
		cp "$RK_FIRMWARE_DIR/boot.img" u-boot/
	fi

	if [ "$RK_SECURITY" ]; then
		echo "Try to build init for $RK_SECURITY_CHECK_METHOD"

		if [ "$RK_SECURITY_CHECK_METHOD" = "DM-V" ]; then
			SYSTEM_IMG=rootfs.squashfs
		else
			SYSTEM_IMG=$ROOTFS_IMG
		fi
		if [ ! -f "$ROOTFS_DIR/$SYSTEM_IMG" ]; then
			echo "There's no $SYSTEM_IMG generated..."
			exit -1
		fi

		"$SCRIPTS_DIR/mk-dm.sh" $RK_SECURITY_CHECK_METHOD \
			"$ROOTFS_DIR/$SYSTEM_IMG"
		ln -rsf "$ROOTFS_DIR/security-system.img" \
			"$RK_FIRMWARE_DIR/rootfs.img"
	fi

	finish_build build_rootfs $@
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

case "${1:-rootfs}" in
	buildroot-config) pre_build_hook $@ ;;
	*) build_hook $@ ;;
esac
