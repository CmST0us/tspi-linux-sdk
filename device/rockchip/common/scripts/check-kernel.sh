#!/bin/bash -e

SCRIPTS_DIR="${SCRIPTS_DIR:-$(dirname "$(realpath "$0")")}"
SDK_DIR="${SDK_DIR:-$SCRIPTS_DIR/../../../..}"

cd "$SDK_DIR"

if [ -r "kernel/.config" ]; then
	EXT4_CONFIGS=$(export | grep -oE "\<RK_.*=\"ext4\"$" || true)

	if [ "$EXT4_CONFIGS" ] && \
		! grep -q "CONFIG_EXT4_FS=y" kernel/.config; then
		echo -e "\e[35m"
		echo "Your kernel doesn't support ext4 filesystem"
		echo "Please enable CONFIG_EXT4_FS for:"
		echo "$EXT4_CONFIGS"
		echo -e "\e[0m"
		exit 1
	fi
fi

if ! kernel/scripts/mkbootimg &>/dev/null; then
	echo -e "\e[35m"
	echo "Your python3 is too old for kernel: $(python3 --version)"
	echo "Please update it:"
	"$SCRIPTS_DIR/python3-install.sh"
	echo -e "\e[0m"
	exit 1
fi
