#!/bin/bash -e

MAKE="make ${DEBUG:+V=1} -C $(realpath --relative-to="$SDK_DIR" "$COMMON_DIR")"

switch_defconfig()
{
	DEFCONFIG="$1"

	[ -f "$DEFCONFIG" ] || DEFCONFIG="$CHIP_DIR/$DEFCONFIG"

	if [ ! -f "$DEFCONFIG" ]; then
		echo "No such defconfig: $1"
		exit 1
	fi

	echo "Switching to defconfig: $DEFCONFIG"
	rm -f "$RK_DEFCONFIG_LINK"
	ln -rsf "$DEFCONFIG" "$RK_DEFCONFIG_LINK"

	DEFCONFIG="$(realpath "$DEFCONFIG")"
	rm -rf "$CHIP_DIR"
	ln -rsf "$(dirname "$DEFCONFIG")" "$CHIP_DIR"

	$MAKE $(basename "$DEFCONFIG")
	exit 0
}

rockchip_defconfigs()
{
	cd "$CHIP_DIR"
	ls rockchip_defconfig 2>/dev/null || true
	ls *_defconfig | grep -v rockchip_defconfig || true
}

choose_defconfig()
{
	DEFCONFIG_ARRAY=( $(rockchip_defconfigs | grep "$1" || true) )

	DEFCONFIG_ARRAY_LEN=${#DEFCONFIG_ARRAY[@]}

	case $DEFCONFIG_ARRAY_LEN in
		0)
			echo "No available defconfigs${1:+" for: $1"}"
			return 1
			;;
		1)	DEFCONFIG=${DEFCONFIG_ARRAY[0]} ;;
		*)
			if [ "$1" = ${DEFCONFIG_ARRAY[0]} ]; then
				# Prefer exact-match
				DEFCONFIG="$1"
			else
				echo "Pick a defconfig:"
				echo ""

				echo ${DEFCONFIG_ARRAY[@]} | xargs -n 1 | \
					sed "=" | sed "N;s/\n/. /"

				local INDEX
				read -p "Which would you like? [1]: " INDEX
				INDEX=$((${INDEX:-1} - 1))
				DEFCONFIG="${DEFCONFIG_ARRAY[$INDEX]}"
			fi
			;;
	esac

	switch_defconfig $DEFCONFIG
}

choose_chip()
{
	CHIP_ARRAY=( $(ls "$CHIPS_DIR" | grep "$1" || true) )
	CHIP_ARRAY_LEN=${#CHIP_ARRAY[@]}

	case $CHIP_ARRAY_LEN in
		0)
			echo "No available chips${1:+" for: $1"}"
			return 1
			;;
		1)	CHIP=${CHIP_ARRAY[0]} ;;
		*)
			if [ "$1" = ${CHIP_ARRAY[0]} ]; then
				# Prefer exact-match
				CHIP="$1"
			else
				echo "Pick a chip:"
				echo ""

				echo ${CHIP_ARRAY[@]} | xargs -n 1 | sed "=" | \
					sed "N;s/\n/. /"

				local INDEX
				read -p "Which would you like? [1]: " INDEX
				INDEX=$((${INDEX:-1} - 1))
				CHIP="${CHIP_ARRAY[$INDEX]}"
			fi
			;;
	esac

	rm -rf "$CHIP_DIR"
	ln -rsf "$CHIPS_DIR/$CHIP" "$CHIP_DIR"

	choose_defconfig $2
}

prepare_config()
{
	[ -e "$CHIP_DIR" ] || choose_chip

	cd "$DEVICE_DIR"
	rm -f $(ls "$CHIPS_DIR")
	ln -rsf "$(readlink "$CHIP_DIR")" .
	cd "$SDK_DIR"

	if [ ! -r "$RK_DEFCONFIG_LINK" ]; then
		echo "WARN: $RK_DEFCONFIG_LINK not exists"
		choose_defconfig
		return 0
	fi

	DEFCONFIG=$(basename "$(realpath "$RK_DEFCONFIG_LINK")")
	if [ ! "$RK_DEFCONFIG_LINK" -ef "$CHIP_DIR/$DEFCONFIG" ]; then
		echo "WARN: $RK_DEFCONFIG_LINK is invalid"
		choose_defconfig
		return 0
	fi

	if [ "$RK_CONFIG" -ot "$RK_DEFCONFIG_LINK" ]; then
		echo "WARN: $RK_CONFIG is out-dated"
		$MAKE $DEFCONFIG &>/dev/null
		return 0
	fi

	CONFIG_DIR="$(dirname "$RK_CONFIG_IN")"
	if find "$CONFIG_DIR" -cnewer "$RK_CONFIG" | grep -q ""; then
		echo "WARN: $CONFIG_DIR is updated"
		$MAKE $DEFCONFIG &>/dev/null
		return 0
	fi

	CFG="RK_DEFCONFIG=\"$(realpath "$RK_DEFCONFIG_LINK")\""
	if ! grep -wq "$CFG" "$RK_CONFIG"; then
		echo "WARN: $RK_CONFIG is invalid"
		$MAKE $DEFCONFIG &>/dev/null
		return 0
	fi

	$MAKE olddefconfig &>/dev/null
}

# Hooks

usage_hook()
{
	echo -e "chip[:<chip>[:<config>]]          \tchoose chip"
	echo -e "defconfig[:<config>]              \tchoose defconfig"
	echo -e " *_defconfig                      \tswitch to specified defconfig"
	echo "    available defconfigs:"
	echo "$(ls "$CHIP_DIR/" | grep "defconfig$" | sed "s/^/\t/")"
	echo -e " olddefconfig                     \tresolve any unresolved symbols in .config"
	echo -e " savedefconfig                    \tsave current config to defconfig"
	echo -e " menuconfig                       \tinteractive curses-based configurator"
	echo -e "config                       \tmodify SDK defconfig"
}

clean_hook()
{
	$MAKE distclean
}

INIT_CMDS="chip defconfig lunch .*_defconfig olddefconfig savedefconfig menuconfig config default"
init_hook()
{
	case "${1:-default}" in
		chip) shift; choose_chip $@ ;;
		lunch|defconfig) shift; choose_defconfig $@ ;;
		*_defconfig) switch_defconfig "$1" ;;
		olddefconfig | savedefconfig | menuconfig) $MAKE $1 ;;
		config)
			$MAKE menuconfig
			$MAKE savedefconfig
			;;
		default) prepare_config ;; # End of init
		*) usage ;;
	esac
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

init_hook $@
