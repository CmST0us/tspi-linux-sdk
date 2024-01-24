#!/bin/bash -e

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

OS_RELEASE="$TARGET_DIR/etc/os-release"

fixup_os_release()
{
	KEY=$1
	shift

	sed -i "/^$KEY=/d" "$OS_RELEASE"
	echo "$KEY=\"$@\"" >> "$OS_RELEASE"
}

echo "Adding information to /etc/os-release..."

mkdir -p "$(dirname "$OS_RELEASE")"
[ -f "$OS_RELEASE" ] || touch "$OS_RELEASE"

fixup_os_release BUILD_INFO "$(whoami)@$(hostname) $(date)${@:+ - $@}"
fixup_os_release KERNEL "$RK_KERNEL_VERSION - ${RK_KERNEL_CFG:-unkown}"

if [ "$POST_OS" != recovery -a "$POST_OS" != pcba ]; then
	cp -f "$OS_RELEASE" "$RK_OUTDIR"
fi
