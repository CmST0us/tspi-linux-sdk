#!/bin/bash -e

if [ -z "${BASH_SOURCE}" ];then
	echo Not in bash, switching to it...
	bash -c "$0 $@"
fi

function choose_board()
{
	echo
	echo "You're building on Linux"
	echo "Lunch menu...pick a combo:"
	echo ""

	echo "0. non-rockchip boards"
	echo ${RK_DEFCONFIG_ARRAY[@]} | xargs -n 1 | sed "=" | sed "N;s/\n/. /"

	local INDEX
	while true; do
		read -p "Which would you like? [0]: " INDEX
		INDEX=$((${INDEX:-0} - 1))

		if [ "$INDEX" -eq -1 ]; then
			echo "Lunching for non-rockchip boards..."
			unset TARGET_OUTPUT_DIR
			unset RK_BUILD_CONFIG
			break;
		fi

		if echo $INDEX | grep -vq [^0-9]; then
			RK_BUILD_CONFIG="${RK_DEFCONFIG_ARRAY[$INDEX]}"
			[ -n "$RK_BUILD_CONFIG" ] && break
		fi

		echo
		echo "Choice not available. Please try again."
		echo
	done
}

function lunch_rockchip()
{
	TARGET_DIR_NAME="$RK_BUILD_CONFIG"
	export TARGET_OUTPUT_DIR="$BUILDROOT_OUTPUT_DIR/$TARGET_DIR_NAME"

	mkdir -p $TARGET_OUTPUT_DIR || return

	echo "==========================================="
	echo
	echo "#TARGET_BOARD=`echo $RK_BUILD_CONFIG | cut -d '_' -f 2`"
	echo "#OUTPUT_DIR=output/$TARGET_DIR_NAME"
	echo "#CONFIG=${RK_BUILD_CONFIG}_defconfig"
	echo
	echo "==========================================="

	make -C ${BUILDROOT_DIR} O="$TARGET_OUTPUT_DIR" \
		"$RK_BUILD_CONFIG"_defconfig

	CONFIG=${TARGET_OUTPUT_DIR}/.config
	cp ${CONFIG}{,.new}
	mv ${CONFIG}{.old,} &>/dev/null || return 0

	make -C ${BUILDROOT_DIR} O="$TARGET_OUTPUT_DIR" olddefconfig &>/dev/null

	if ! diff ${CONFIG}{,.new}; then
		read -t 10 -p "Found old config, override it? (y/n):" YES
		[ "$YES" = "n" ] || cp ${CONFIG}{.new,}
	fi
}

function main()
{
	SCRIPT_PATH=$(realpath ${BASH_SOURCE})
	SCRIPT_DIR=$(dirname ${SCRIPT_PATH})
	BUILDROOT_DIR=$(dirname ${SCRIPT_DIR})
	BUILDROOT_OUTPUT_DIR=${BUILDROOT_DIR}/output
	TOP_DIR=$(dirname ${BUILDROOT_DIR})
	echo Top of tree: ${TOP_DIR}

	RK_DEFCONFIG_ARRAY=(
		$(cd ${BUILDROOT_DIR}/configs/; ls rockchip_* | \
			sed "s/_defconfig$//" | grep "$1" | sort)
	)

	unset RK_BUILD_CONFIG
	RK_DEFCONFIG_ARRAY_LEN=${#RK_DEFCONFIG_ARRAY[@]}

	case $RK_DEFCONFIG_ARRAY_LEN in
		0)
			echo No available configs${1:+" for: $1"}
			;;
		1)
			RK_BUILD_CONFIG=${RK_DEFCONFIG_ARRAY[0]}
			;;
		*)
			if [ "$1" = ${RK_DEFCONFIG_ARRAY[0]} ]; then
				# Prefer exact-match
				RK_BUILD_CONFIG=$1
			else
				choose_board
			fi
			;;
	esac

	[ -n "$RK_BUILD_CONFIG" ] || return

	source ${TOP_DIR}/device/rockchip/.BoardConfig.mk

	lunch_rockchip

	# Set alias
	alias croot="cd ${TOP_DIR}"
	alias broot="cd ${BUILDROOT_DIR}"
	alias bpkg="cd ${BUILDROOT_DIR}/package"
	alias bout="cd ${TARGET_OUTPUT_DIR}"
}

if [ "${BASH_SOURCE}" == "$0" ];then
	echo This script is executed directly...
	bash -c "source \"$0\" \"$@\"; bash"
else
	main "$@"
fi
