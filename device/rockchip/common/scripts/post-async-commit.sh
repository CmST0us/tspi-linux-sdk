#!/bin/bash -e

POST_OS_DISALLOWED="recovery pcba"

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

echo "Installing async-commit service..."

rm -f etc/init.d/S10async_commit.sh \
	etc/systemd/system/multi-user.target.wants/async.service \
	usr/lib/systemd/system/async.service

cd "$SDK_DIR"

mkdir -p "$TARGET_DIR/usr/bin"
install -m 0755 external/rkscript/async-commit "$TARGET_DIR/usr/bin/"

if [ "$POST_INIT_SYSTEMD" ]; then
	install -m 0755 external/rkscript/async-commit.service \
		"$TARGET_DIR/lib/systemd/system/"
	mkdir -p "$TARGET_DIR/etc/systemd/system/sysinit.target.wants"
	ln -sf /lib/systemd/system/async-commit.service \
		"$TARGET_DIR/etc/systemd/system/sysinit.target.wants/"
fi

if [ "$POST_INIT_SYSV" ]; then
	install -m 0755 external/rkscript/S10async-commit.sh \
		"$TARGET_DIR/etc/init.d/async-commit.sh"
	ln -sf ../init.d/${TYPE}all.sh \
		"$TARGET_DIR/etc/rcS.d/S10async-commit.sh"
fi

if [ "$POST_INIT_BUSYBOX" ]; then
	install -m 0755 external/rkscript/S10async-commit.sh \
		"$TARGET_DIR/etc/init.d/"
fi
