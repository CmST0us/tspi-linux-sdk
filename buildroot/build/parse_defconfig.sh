#!/bin/bash
#
# Usage:
# parse_defconfig.sh <defconfig> [output config path]

set -e

parse_includes()
{
	sed -n "/^#include /s/.*\"\(.*\)\"/configs\/rockchip\/\1/p" $@
}

extract_config()
{
	for config in $(parse_includes $@); do
		extract_config $config
	done
	echo "$1"
}

SCRIPT_DIR="$(realpath "$(dirname "$0")")"
BUILDROOT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$BUILDROOT_DIR"

BOARD="$(basename "${1%_defconfig}")"
if [ -r "$1" ]; then
	DEFCONFIG="$1"

	# Unknown board
	echo "$DEFCONFIG" | grep -q "_defconfig$" || unset BOARD
else
	DEFCONFIG="configs/${BOARD}_defconfig"
	if [ ! -r "$DEFCONFIG" ]; then
		echo "Unable to locate defconfig: $@"
		exit 1
	fi
fi

export KCONFIG_CONFIG="${2:-$BUILDROOT_DIR/output/$BOARD/.config}"

echo "Parsing defconfig: $DEFCONFIG"

"$SCRIPT_DIR/merge_config.sh" -m -O "$(dirname $KCONFIG_CONFIG)" \
	$(extract_config "$DEFCONFIG")

sed -i "s~\(^BR2_DEFCONFIG=\).*~\1\"$(realpath "$DEFCONFIG")\"~" \
	"$KCONFIG_CONFIG"
