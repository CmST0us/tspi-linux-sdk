#!/bin/bash
unset CPLUS_INCLUDE_PATH
unset LIBRARY_PATH

if [ -z "$BASH_SOURCE" ]; then
	echo "Not in bash, switching to it..."
	case "${@:-shell}" in
		shell) ./build.sh shell ;;
		*)
			./build.sh $@
			bash
			;;
	esac
fi

usage()
{
	echo "Usage: $(basename $BASH_SOURCE) [OPTIONS]"
	echo "Available options:"

	run_build_hooks usage

	# Global options
	echo -e "cleanall                          \tcleanup"
	echo -e "post-rootfs <rootfs dir>          \ttrigger post-rootfs hook scripts"
	echo -e "shell                             \tsetup a shell for developing"
	echo -e "help                              \tusage"
	echo ""
	echo "Default option is 'allsave'."
	exit 0
}

err_handler()
{
	ret=${1:-$?}
	[ "$ret" -eq 0 ] && return

	echo "ERROR: Running $BASH_SOURCE - ${2:-${FUNCNAME[1]}} failed!"
	echo "ERROR: exit code $ret from line ${BASH_LINENO[0]}:"
	echo "    ${3:-$BASH_COMMAND}"
	echo "ERROR: call stack:"
	for i in $(seq 1 $((${#FUNCNAME[@]} - 1))); do
		SOURCE="${BASH_SOURCE[$i]}"
		LINE=${BASH_LINENO[$(( $i - 1 ))]}
		echo "    $(basename "$SOURCE"): ${FUNCNAME[$i]}($LINE)"
	done
	exit $ret
}

# Export global functions
set -a

finish_build()
{
	echo -e "\e[35mRunning $(basename "${BASH_SOURCE[1]}") - ${@:-${FUNCNAME[1]}} succeeded.\e[0m"
	cd "$SDK_DIR"
}

check_config()
{
	unset missing
	for var in $@; do
		eval [ \$$var ] && continue

		missing="$missing $var"
	done

	[ -z "$missing" ] && return 0

	echo "Skipping $(basename "${BASH_SOURCE[1]}") - ${FUNCNAME[1]} for missing configs: $missing."
	return 1
}

kernel_version_real()
{
	[ -d kernel ] || return 0

	VERSION_KEYS="VERSION PATCHLEVEL"
	VERSION=""

	for k in $VERSION_KEYS; do
		v=$(grep "^$k = " kernel/Makefile | cut -d' ' -f3)
		VERSION=${VERSION:+${VERSION}.}$v
	done
	echo $VERSION
}

kernel_version()
{
	[ -d kernel ] || return 0

	KERNEL_DIR="$(basename "$(realpath kernel)")"
	case "$KERNEL_DIR" in
		kernel-*)
			echo ${KERNEL_DIR#kernel-}
			return 0
			;;
	esac

	kernel_version_real
}

start_log()
{
	LOG_FILE="$RK_LOG_DIR/${2:-$1_$(date +%F_%H-%M-%S)}.log"
	ln -rsf "$LOG_FILE" "$RK_LOG_DIR/$1.log"
	echo "# $(date +"%F %T")" >> "$LOG_FILE"
	echo "$LOG_FILE"
}

# For developing shell only

rroot()
{
	cd "$SDK_DIR"
}

rout()
{
	cd "$RK_OUTDIR"
}

rcommon()
{
	cd "$COMMON_DIR"
}

rscript()
{
	cd "$SCRIPTS_DIR"
}

rchip()
{
	cd "$(realpath "$CHIP_DIR")"
}

set +a
# End of global functions

run_hooks()
{
	DIR="$1"
	shift

	for dir in "$CHIP_DIR/$(basename "$DIR")/" "$DIR"; do
		[ -d "$dir" ] || continue

		for hook in $(find "$dir" -maxdepth 1 -name "*.sh" | sort); do
			"$hook" $@ && continue
			HOOK_RET=$?
			err_handler $HOOK_RET "${FUNCNAME[0]} $*" "$hook $*"
			exit $HOOK_RET
		done
	done
}

run_build_hooks()
{
	# Don't log these hooks
	case "$1" in
		init | pre-build | usage | support-cmds)
			run_hooks "$RK_BUILD_HOOK_DIR" $@ || true
			return 0
			;;
	esac

	LOG_FILE="$(start_log "$1")"

	echo -e "# run hook: $@\n" >> "$LOG_FILE"
	run_hooks "$RK_BUILD_HOOK_DIR" $@ 2>&1 | tee -a "$LOG_FILE"
	HOOK_RET=${PIPESTATUS[0]}
	if [ $HOOK_RET -ne 0 ]; then
		err_handler $HOOK_RET "${FUNCNAME[0]} $*" "$@"
		exit $HOOK_RET
	fi
}

run_post_hooks()
{
	LOG_FILE="$(start_log post-rootfs)"

	echo -e "# run hook: $@\n" >> "$LOG_FILE"
	run_hooks "$RK_POST_HOOK_DIR" $@ 2>&1 | tee -a "$LOG_FILE"
	HOOK_RET=${PIPESTATUS[0]}
	if [ $HOOK_RET -ne 0 ]; then
		err_handler $HOOK_RET "${FUNCNAME[0]} $*" "$@"
		exit $HOOK_RET
	fi
}

option_check()
{
	CMDS="$1"
	shift

	for opt in $@; do
		for cmd in $CMDS; do
			# NOTE: There might be patterns in commands
			echo "${opt%%:*}" | grep -q "^$cmd$" || continue
			return 0
		done
	done

	return 1
}

main()
{
	[ -z "$DEBUG" ] || set -x

	trap 'err_handler' ERR
	set -eE

	# Save intial envionments
	INITIAL_ENV=$(mktemp -u)
	if [ -z "$RK_SESSION" ]; then
		env > "$INITIAL_ENV"
	fi

	export LC_ALL=C

	export SCRIPTS_DIR="$(dirname "$(realpath "$BASH_SOURCE")")"
	export COMMON_DIR="$(realpath "$SCRIPTS_DIR/..")"
	export SDK_DIR="$(realpath "$COMMON_DIR/../../..")"
	export DEVICE_DIR="$SDK_DIR/device/rockchip"
	export CHIPS_DIR="$DEVICE_DIR/.chips"
	export CHIP_DIR="$DEVICE_DIR/.chip"

	export RK_DATA_DIR="$COMMON_DIR/data"
	export RK_IMAGE_DIR="$COMMON_DIR/images"
	export RK_CONFIG_IN="$COMMON_DIR/configs/Config.in"

	export RK_BUILD_HOOK_DIR="$COMMON_DIR/build-hooks"
	export BUILD_HELPER="$RK_BUILD_HOOK_DIR/build-helper"
	export RK_POST_HOOK_DIR="$COMMON_DIR/post-hooks"
	export POST_HELPER="$RK_POST_HOOK_DIR/post-helper"

	export PARTITION_HELPER="$SCRIPTS_DIR/partition-helper"

	export RK_OUTDIR="$SDK_DIR/output"
	export RK_LOG_BASE_DIR="$RK_OUTDIR/log"
	export RK_SESSION="${RK_SESSION:-$(date +%F_%H-%M-%S)}"
	export RK_LOG_DIR="$RK_LOG_BASE_DIR/$RK_SESSION"
	export RK_FIRMWARE_DIR="$RK_OUTDIR/firmware"
	export RK_INITIAL_ENV="$RK_OUTDIR/initial.env"
	export RK_CUSTOM_ENV="$RK_OUTDIR/custom.env"
	export RK_FINAL_ENV="$RK_OUTDIR/final.env"
	export RK_CONFIG="$RK_OUTDIR/.config"
	export RK_DEFCONFIG_LINK="$RK_OUTDIR/defconfig"

	if [ ! -d "$RK_LOG_DIR" ]; then
		mkdir -p "$RK_LOG_DIR"
		rm -rf "$RK_LOG_BASE_DIR/latest"
		ln -rsf "$RK_LOG_DIR" "$RK_LOG_BASE_DIR/latest"
		echo -e "\e[33mLog saved at $RK_LOG_DIR\e[0m"
		echo
	fi

	# Drop old logs
	cd "$RK_LOG_BASE_DIR"
	rm -rf $(ls -t | sed '1,10d')

	mkdir -p "$RK_FIRMWARE_DIR"
	rm -rf "$SDK_DIR/rockdev"
	ln -rsf "$RK_FIRMWARE_DIR" "$SDK_DIR/rockdev"

	cd "$SDK_DIR"
	[ -f README.md ] || ln -rsf "$COMMON_DIR/README.md" .

	# TODO: Remove it in the repo manifest.xml
	rm -f envsetup.sh

	OPTIONS="${@:-allsave}"

	# For Makefile parsing script targets
	if [ "$OPTIONS" = "core-usage" ]; then
		run_build_hooks usage
		exit 0
	fi

	# For rpdzkj link init to chip option
        if [ "$OPTIONS" = "init" ]; then
		OPTIONS=chip
        fi

	# Options checking
	CMDS="$(run_build_hooks support-cmds all | xargs)"
	for opt in $OPTIONS; do
		case "$opt" in
			help | h | -h | --help | usage | \?) usage ;;
			shell | cleanall)
				# Check single options
				if [ "$opt" = "$OPTIONS" ]; then
					break
				fi

				echo "ERROR: $opt cannot combine with other options!"
				;;
			post-rootfs)
				if [ "$opt" = "$1" -a -d "$2" ]; then
					# Hide other args from build stages
					OPTIONS=$opt
					break
				fi

				echo "ERROR: $opt should be the first option followed by rootfs dir!"
				;;
			*)
				# Make sure that all options are handled
				if option_check "$CMDS" $opt; then
					continue
				fi

				echo "ERROR: Unhandled option: $opt"
				;;
		esac

		usage
	done

	# Init stage (preparing SDK configs, etc.)
	run_build_hooks init $OPTIONS
	rm -f "$RK_OUTDIR/.tmpconfig*"

	# No need to go further
	CMDS="$(run_build_hooks support-cmds pre-build build \
		post-build | xargs) shell cleanall post-rootfs"
	option_check "$CMDS" $OPTIONS || return 0

	# Force exporting config environments
	set -a

	# Load config environments
	source "$RK_CONFIG"
	cp "$RK_CONFIG" "$RK_LOG_DIR"

	# Save initial environment
	if [ -e "$INITIAL_ENV" ]; then
		cat "$INITIAL_ENV" > "$RK_INITIAL_ENV"
		rm -f "$RK_CUSTOM_ENV"

		# Find custom environments
		for cfg in $(grep "^RK_" "$RK_INITIAL_ENV" || true); do
			env | grep -q "^$cfg$" || \
				echo "$cfg" >> "$RK_CUSTOM_ENV"
		done

		# Allow custom environments overriding
		if [ -e "$RK_CUSTOM_ENV" ]; then
			echo -e "\e[31mWARN: Found custom environments: \e[0m"
			cat "$RK_CUSTOM_ENV"

			echo -e "\e[31mAssuming that is expected, please clear them if otherwise.\e[0m"
			read -t 10 -p "Press enter to continue."
			source "$RK_CUSTOM_ENV"
			cp "$RK_CUSTOM_ENV" "$RK_LOG_DIR"
		fi
	fi

	source "$PARTITION_HELPER"
	rk_partition_init

	set +a

	# RV1126 uses custom toolchain
	if [ "$RK_CHIP_FAMILY" = "rv1126_rv1109" ]; then
		TOOLCHAIN_OS=rockchip
	else
		TOOLCHAIN_OS=none
	fi

	TOOLCHAIN_ARCH=${RK_KERNEL_ARCH/arm64/aarch64}
	TOOLCHAIN_DIR="$(realpath prebuilts/gcc/*/$TOOLCHAIN_ARCH)"
	GCC="$(find "$TOOLCHAIN_DIR" -name "*$TOOLCHAIN_OS*-gcc" | head -n 1)"
	if [ ! -x "$GCC" ]; then
		echo "No prebuilt GCC toolchain!"
		exit 1
	fi

	export RK_TOOLCHAIN="${GCC%gcc}"
	echo "Prebuilt toolchain (for kernel & loader):"
	echo "$RK_TOOLCHAIN"

	export PYTHON3=/usr/bin/python3

	if [ "$RK_KERNEL_CFG" ]; then
		CPUS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
		export KMAKE="make -C kernel/ -j$(( $CPUS + 1 )) \
			CROSS_COMPILE=$RK_TOOLCHAIN ARCH=$RK_KERNEL_ARCH"
		export RK_KERNEL_VERSION_REAL=$(kernel_version_real)
	fi

	# Handle special commands
	case "$OPTIONS" in
		shell)
			echo -e "\e[35mDoing this is dangerous and for developing only.\e[0m"
			# No error handling in develop shell.
			set +e; trap ERR
			/bin/bash
			echo -e "\e[35mExit from $BASH_SOURCE shell.\e[0m"
			exit 0 ;;
		cleanall)
			run_build_hooks clean
			rm -rf "$RK_OUTDIR"
			finish_build cleanall
			exit 0 ;;
		post-rootfs)
			shift
			run_post_hooks $@
			finish_build post-rootfs
			exit 0 ;;
	esac

	# Save final environments
	env > "$RK_FINAL_ENV"
	cp "$RK_FINAL_ENV" "$RK_LOG_DIR"

	# Log configs
	echo
	echo "=========================================="
	echo "          Final configs"
	echo "=========================================="
	env | grep -E "^RK_.*=.+" | grep -vE "PARTITION_[0-9]" | \
		grep -vE "=\"\"$|_DEFAULT=y" | \
		grep -vE "^RK_CONFIG|_BASE_CFG=|_LINK=|DIR=|_ENV=|_NAME=" | sort
	echo

	# Pre-build stage (submodule configuring, etc.)
	run_build_hooks pre-build $OPTIONS

	# No need to go further
	CMDS="$(run_build_hooks support-cmds build post-build | xargs)"
	option_check "$CMDS" $OPTIONS || return 0

	# Build stage (building, etc.)
	run_build_hooks build $OPTIONS

	# No need to go further
	CMDS="$(run_build_hooks support-cmds post-build | xargs)"
	option_check "$CMDS" $OPTIONS || return 0

	# Post-build stage (firmware packing, etc.)
	run_build_hooks post-build $OPTIONS
}

if [ "$0" != "$BASH_SOURCE" ]; then
	# Sourced, executing it directly
	"$BASH_SOURCE" ${@:-shell}
elif [ "$0" == "$BASH_SOURCE" ]; then
	# Executed directly
	main $@
fi
