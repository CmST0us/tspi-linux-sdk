#!/bin/bash -e

KERNELS=$(ls | grep kernel- || true)

update_kernel()
{
	# Fallback to current kernel
	RK_KERNEL_VERSION=${RK_KERNEL_VERSION:-$(kernel_version)}

	# Fallback to 5.10 kernel
	RK_KERNEL_VERSION=${RK_KERNEL_VERSION:-5.10}

	sed -i "s/^\(RK_KERNEL_VERSION=\).*/\1\"$RK_KERNEL_VERSION\"/" \
		"$RK_CONFIG"

	[ "$(kernel_version)" != "$RK_KERNEL_VERSION" ] || return 0

	# Update kernel
	KERNEL_DIR=kernel-$RK_KERNEL_VERSION
	echo "switching to $KERNEL_DIR"
	if [ ! -d "$KERNEL_DIR" ]; then
		echo "$KERNEL_DIR not exist!"
		exit 1
	fi

	rm -rf kernel
	ln -rsf $KERNEL_DIR kernel
}

do_build()
{
	if [ "$DRY_RUN" ]; then
		echo -e "\e[35mCommands of building $1:\e[0m"
	else
		echo "=========================================="
		echo "          Start building $1"
		echo "=========================================="
	fi

	check_config RK_KERNEL_DTS_NAME RK_KERNEL_CFG RK_BOOT_IMG || return 0

	if [ ! "$DRY_RUN" ]; then
		"$SCRIPTS_DIR/check-kernel.sh"
	fi

	run_command $KMAKE $RK_KERNEL_CFG $RK_KERNEL_CFG_FRAGMENTS

	case "$1" in
		kernel-config)
			KERNEL_CONFIG_DIR="kernel/arch/$RK_KERNEL_ARCH/configs"
			run_command $KMAKE menuconfig
			run_command $KMAKE savedefconfig
			run_command mv kernel/defconfig \
				"$KERNEL_CONFIG_DIR/$RK_KERNEL_CFG"
			;;
		kernel*)
			run_command $KMAKE "$RK_KERNEL_DTS_NAME.img"

			# The FIT image for initrd would be packed in rootfs stage
			if [ -n "$RK_BOOT_FIT_ITS" ]; then
				if [ -z "$RK_ROOTFS_INITRD" ]; then
					run_command \
						"$SCRIPTS_DIR/mk-fitimage.sh" \
						"kernel/$RK_BOOT_IMG" \
						"$RK_BOOT_FIT_ITS" \
						"$RK_KERNEL_IMG"
				fi
			fi
			;;
		modules) run_command $KMAKE modules ;;
	esac
}

# Hooks

usage_hook()
{
	for k in $KERNELS; do
		echo -e "$k[:cmds]               \tbuild kernel ${k#kernel-}"
	done

	echo -e "kernel[:cmds]                    \tbuild kernel"
	echo -e "modules[:cmds]                   \tbuild kernel modules"
	echo -e "linux-headers[:cmds]             \tbuild linux-headers"
	echo -e "kernel-config[:cmds]             \tmodify kernel defconfig"
}

clean_hook()
{
	make -C kernel distclean
}

INIT_CMDS="default $KERNELS"
init_hook()
{
	case "$1" in
		kernel-*) export RK_KERNEL_VERSION=${1#kernel-} ;&
		*) update_kernel ;;
	esac
}

PRE_BUILD_CMDS="kernel-config"
pre_build_hook()
{
	check_config RK_KERNEL_CFG || return 0
	do_build $@

	finish_build $@
}

pre_build_hook_dry()
{
	DRY_RUN=1 pre_build_hook $@
}

BUILD_CMDS="$KERNELS kernel modules"
build_hook()
{
	check_config RK_KERNEL_DTS_NAME RK_KERNEL_CFG RK_BOOT_IMG || return 0

	if echo $1 | grep -q "^kernel-"; then
		if [ "$RK_KERNEL_VERSION" != "${1#kernel-}" ]; then
			echo -ne "\e[35m"
			echo "Kernel version overrided: " \
				"$RK_KERNEL_VERSION -> ${1#kernel-}"
			echo -ne "\e[0m"
		fi
	fi

	do_build $@

	[ ! "$DRY_RUN" ] || return 0

	if echo $1 | grep -q "^kernel"; then
		ln -rsf "kernel/$RK_BOOT_IMG" "$RK_FIRMWARE_DIR/boot.img"

		[ -z "$RK_SECURITY" ] || cp "$RK_FIRMWARE_DIR/boot.img" u-boot/

		"$SCRIPTS_DIR/check-power-domain.sh"
	fi

	finish_build build_$1
}

build_hook_dry()
{
	DRY_RUN=1 build_hook $@
}

POST_BUILD_CMDS="linux-headers"
post_build_hook()
{
	check_config RK_KERNEL_DTS_NAME RK_KERNEL_CFG RK_BOOT_IMG || return 0

	OUTPUT_DIR="${2:-"$RK_OUTDIR/linux-headers"}"
	OUTPUT_DIR="$(realpath "$OUTPUT_DIR")"
	HEADER_FILES_SCRIPT="$OUTPUT_DIR/.header-files.sh"

	if [ "$DRY_RUN" ]; then
		echo -e "\e[35mCommands of building $1:\e[0m"
	else
		echo "Saving linux-headers to $OUTPUT_DIR"
	fi

	rm -rf "$OUTPUT_DIR"
	mkdir -p "$OUTPUT_DIR"

	run_command cd kernel

	cat << EOF > "$HEADER_FILES_SCRIPT"
{
	# Based on kernel/scripts/package/builddeb
	find . arch/$RK_KERNEL_ARCH -maxdepth 1 -name Makefile\*
	find include scripts -type f -o -type l
	find arch/$RK_KERNEL_ARCH -name module.lds -o -name Kbuild.platforms -o -name Platform
	find \$(find arch/$RK_KERNEL_ARCH -name include -o -name scripts -type d) -type f
	find arch/$RK_KERNEL_ARCH/include Module.symvers include scripts -type f
	echo .config
} | rsync -a --files-from=- . "$OUTPUT_DIR"
EOF

	cat "$HEADER_FILES_SCRIPT"

	if [ -z "$DRY_RUN" ]; then
		. "$HEADER_FILES_SCRIPT"
	fi

	run_command cd "$SDK_DIR"
}

post_build_hook_dry()
{
	DRY_RUN=1 post_build_hook $@
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

case "${1:-kernel}" in
	kernel-config) pre_build_hook $@ ;;
	kernel* | modules)
		init_hook $@
		build_hook ${@:-kernel}
		;;
	linux-headers) post_build_hook $@ ;;
	*) usage ;;
esac
