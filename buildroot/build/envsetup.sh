#!/bin/bash -e

if [ -z "$BASH_SOURCE" ];then
	echo "Not in bash, switching to it..."
	"$(find . -maxdepth 3 -name envsetup.sh | head -n 1 || echo /bin/bash)"
fi

choose_board()
{
	echo
	echo "Pick a board:"
	echo ""

	echo ${RK_BOARD_ARRAY[@]} | xargs -n 1 | sed "=" | sed "N;s/\n/. /"

	local INDEX
	while true; do
		read -p "Which would you like? [1]: " INDEX
		INDEX=$((${INDEX:-1} - 1))

		if echo $INDEX | grep -vq [^0-9]; then
			RK_BOARD="${RK_BOARD_ARRAY[$INDEX]}"
			[ -z "$RK_BOARD" ] || break
		fi

		echo
		echo "Choice not available. Please try again."
		echo
	done
}

setup_board()
{
	export BUILDROOT_OUTPUT_DIR="$BUILDROOT_DIR/output/$RK_BOARD"
	BMAKE="make -C $BUILDROOT_DIR O=$BUILDROOT_OUTPUT_DIR"

	if [ $RK_BOARD_ARRAY_LEN -eq 0 ]; then
		echo "Continue without defconfig..."
		${BMAKE} olddefconfig &>/dev/null
		return 0
	fi

	${BMAKE} "${RK_BOARD}_defconfig"

	CONFIG="$BUILDROOT_OUTPUT_DIR/.config"
	cp "$CONFIG" "$CONFIG.new"
	mv "$CONFIG.old" "$CONFIG" &>/dev/null || return 0

	${BMAKE} olddefconfig &>/dev/null

	if ! diff "$CONFIG" "$CONFIG.new"; then
		read -t 10 -p "Found old config, override it? (y/n):" YES
		[ "$YES" = "n" ] || cp "$CONFIG.new" "$CONFIG"
	fi
}

bpkg()
{
	case "${1:-dir}" in
		dir)
			if [ -n "$2" ]; then
				find "$BUILDROOT_OUTPUT_DIR/build/" -maxdepth 1 \
					-type d -name "*$2*" | head -n 1 || \
					echo "no pkg build dir for $2." >&2
			else
				echo $(realpath "$PWD") | \
					grep -oE ".*/output/[^/]*/build/[^/]*" || \
					echo "not in a pkg build dir." >&2
			fi
			;;
		*)
			bpkg dir $1
			;;
	esac
}

bpkg_run()
{
	DIR=$(bpkg dir $2)
	[ -d "$DIR" ] || return 1

	for stage in $1; do
		case "$stage" in
			reconfig) bpkg_run "configure build install deploy" $2 ;;
			rebuild) bpkg_run "build install deploy" $2 ;;
			reinstall) bpkg_run "install deploy" $2 ;;
			reconfig-update)
				bpkg_run "configure build install update" $2 ;;
			rebuild-update) bpkg_run "build install update" $2 ;;
			reinstall-update) bpkg_run "install update" $2 ;;
			configure|build|deploy|update)
				SCRIPT="$DIR/.$stage.sh"
				[ -x "$SCRIPT" ] || return 1
				"$SCRIPT" || return 1
				;;
			install)
				SCRIPT="$DIR/.staging_install.sh"
				if [ -x "$SCRIPT" ]; then
					"$SCRIPT" || return 1
				fi

				SCRIPT="$DIR/.target_install.sh"
				if [ -x "$SCRIPT" ]; then
					"$SCRIPT" || return 1

					SCRIPT="$DIR/.image_install.sh"
					if [ -x "$SCRIPT" ]; then
						"$SCRIPT" || return 1
					fi

					continue
				fi

				SCRIPT="$DIR/.host_install.sh"
				if [ -x "$SCRIPT" ]; then
					"$SCRIPT" || return 1
					continue
				fi

				return 1
				;;
			*)
				echo "Unknown stage: $stage"
				return 1
				;;
		esac
	done
}

main()
{
	SCRIPTS_DIR="$(dirname "$(realpath "$BASH_SOURCE")")"
	BUILDROOT_DIR="$(dirname "$SCRIPTS_DIR")"
	TOP_DIR="$(dirname "$BUILDROOT_DIR")"
	echo "Top of tree: $TOP_DIR"

	RK_BOARD_ARRAY=(
		$(cd "$BUILDROOT_DIR/configs/"; ls rockchip_* | \
			grep "$(basename "$1")" | sed "s/_defconfig$//" | sort)
	)
	RK_BOARD_ARRAY_LEN=${#RK_BOARD_ARRAY[@]}

	case $RK_BOARD_ARRAY_LEN in
		0)
			# Try existing output without defconfig
			BOARD="$(echo "$1" | \
				sed "s#^\(output/\|\)rockchip_\([^/]*\).*#\2#")"
			RK_BOARD="${BOARD:+rockchip_$BOARD}"
			CONFIG="$BUILDROOT_DIR/output/$RK_BOARD/.config"
			if [ ! -f "$CONFIG" ]; then
				unset RK_BOARD
				echo "No available configs${1:+" for: $1"}"
				return
			fi
			;;
		1)
			RK_BOARD=${RK_BOARD_ARRAY[0]}
			;;
		*)
			if [ "$1" = ${RK_BOARD_ARRAY[0]} ]; then
				# Prefer exact-match
				RK_BOARD="$1"
			else
				choose_board
			fi
			;;
	esac

	setup_board

	# Set alias
	alias croot='cd "$TOP_DIR"'
	alias broot='cd "$BUILDROOT_DIR"'
	alias bmake='make -C "$BUILDROOT_OUTPUT_DIR"'

	alias bconfig='bpkg_run configure'
	alias bbuild='bpkg_run build'
	alias binstall='bpkg_run install'
	alias bdeploy='bpkg_run deploy'
	alias bupdate='bpkg_run update'

	alias breconfig='bpkg_run reconfig'
	alias brebuild='bpkg_run rebuild'
	alias breinstall='bpkg_run reinstall'
	alias breconfig-update='bpkg_run reconfig-update'
	alias brebuild-update='bpkg_run rebuild-update'
	alias breinstall-update='bpkg_run reinstall-update'
}

main "$@"

if [ "$BASH_SOURCE" == "$0" ];then
	# This script is executed directly
	/bin/bash
fi
