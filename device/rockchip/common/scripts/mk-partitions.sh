#!/bin/bash -e

modify_partitions()
{
	echo "=========================================="
	echo "          Start modifying partitions"
	echo "=========================================="

	rk_partition_print
	echo
	echo "Usage:"
	core_usage | grep -v "^mod-parts"

	while true; do
		echo
		read -p "Commands (? for help): " SUB_CMD ARGS || break
		case "${SUB_CMD:-print-parts}" in
			"done") break ;;
			print-parts)
				rk_partition_print
				continue
				;;
			edit-parts) FUNC=rk_partition_edit ;;
			new-parts) FUNC=rk_partition_create ;;
			insert-part) FUNC=rk_partition_insert ;;
			del-part) FUNC=rk_partition_del ;;
			move-part) FUNC=rk_partition_move ;;
			rename-part) FUNC=rk_partition_rename ;;
			resize-part) FUNC=rk_partition_resize ;;
			help | h | -h | --help | \?) FUNC=false ;;
			*)
				echo "Unknown command: $SUB_CMD"
				FUNC=false
				;;
		esac

		if $FUNC $ARGS; then
			rk_partition_print
		else
			core_usage | grep -v "^mod-parts"
			echo -e "done                               \tdone modifying"
		fi
	done
}

# Hooks

usage_hook()
{
	echo -e "print-parts                        \tprint partitions"
	echo -e "mod-parts                          \tinteractive partition table modify"
	echo -e "edit-parts                         \tedit raw partitions"
	echo -e "new-parts:<offset>:<name>:<size>...\tre-create partitions"
	echo -e "insert-part:<idx>:<name>[:<size>]  \tinsert partition"
	echo -e "del-part:(<idx>|<name>)            \tdelete partition"
	echo -e "move-part:(<idx>|<name>):<idx>     \tmove partition"
	echo -e "rename-part:(<idx>|<name>):<name>  \trename partition"
	echo -e "resize-part:(<idx>|<name>):<size>  \tresize partition"
}

PRE_BUILD_CMDS="print-parts mod-parts edit-parts new-parts insert-part del-part move-part rename-part resize-part"
pre_build_hook()
{
	check_config RK_PARAMETER || return 0

	CMD=$1
	shift

	case "$CMD" in
		print-parts) rk_partition_print $@ ;;
		mod-parts) modify_partitions $@ ;;
		edit-parts) rk_partition_edit $@ ;;
		new-parts) rk_partition_create $@ ;;
		insert-part) rk_partition_insert $@ ;;
		del-part) rk_partition_del $@ ;;
		move-part) rk_partition_move $@ ;;
		rename-part) rk_partition_rename $@ ;;
		resize-part) rk_partition_resize $@ ;;
		*)
			core_usage
			exit 1
			;;
	esac

	finish_build $CMD $@
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

pre_build_hook $@
