#!/bin/bash -e

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

LOCALE="$TARGET_DIR/etc/default/locale"

[ -z "$RK_ROOTFS_LOCALE_ORIGINAL" ] || exit 0

if [ "$RK_ROOTFS_LOCALE_DEFAULT" -a "$POST_OS" = debian ]; then
	echo -e "\e[33mKeep original locale for debian by default\e[0m"
	exit 0
fi

CUSTOM_LANG="${RK_ROOTFS_LOCALE:-en_US.UTF-8}"

echo "Setting LANG environment to $CUSTOM_LANG..."

if [ -e "$LOCALE" ]; then
	sed -i "/\<LANG\>/d" "$LOCALE"
	echo "LANG=$CUSTOM_LANG" >> "$LOCALE"
else
	mkdir -p "$TARGET_DIR/etc/profile.d"
	echo "export LANG=$CUSTOM_LANG" > "$TARGET_DIR/etc/profile.d/lang.sh"
fi
