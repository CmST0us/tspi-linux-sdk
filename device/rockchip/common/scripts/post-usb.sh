#!/bin/bash -e

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

install_adbd()
{
	[ -n "$RK_USB_ADBD" ] || return 0

	echo "Installing adbd..."

	find "$TARGET_DIR" -name "*adbd*" -print0 | xargs -0 rm -rf

	install -m 0755 "$RK_DATA_DIR/tools/adbd" "$TARGET_DIR/usr/bin/adbd"

	if [ "$RK_USB_ADBD_TCP_PORT" -ne 0 ]; then
		echo "export ADB_TCP_PORT=$RK_USB_ADBD_TCP_PORT" >> \
			"$TARGET_DIR/etc/profile.d/adbd.sh"
	fi

	if [ -n "$RK_USB_ADBD_BASH" -a -x "$TARGET_DIR/bin/bash" ]; then
		echo "export ADBD_SHELL=/bin/bash" >> \
			"$TARGET_DIR/etc/profile.d/adbd.sh"
	fi

	if [ -n "$RK_USB_ADBD_PASSWORD" ]; then
		install -m 0755 "$RK_DATA_DIR/misc/adbd-auth" \
			"$TARGET_DIR/usr/bin/adbd-auth"
		sed -i "s/AUTH_PASSWORD/$(echo $RK_USB_ADBD_PASSWORD | md5sum)/g" \
			"$TARGET_DIR/usr/bin/adbd-auth"
	fi

	if [ -r "$RK_USB_ADBD_RSA_KEY" ]; then
		install -m 0644 "$RK_USB_ADBD_RSA_KEY" \
			"$TARGET_DIR/etc/adbkey.pub"
		echo "export ADBD_RSA_KEY_FILE=/etc/adbkey.pub" >> \
			"$TARGET_DIR/etc/profile.d/adbd.sh"
	fi
}

install_ums()
{
	[ -n "$RK_USB_UMS" ] || return 0

	echo "Installing UMS..."

	{
		echo "export UMS_FILE=${RK_USB_UMS_FILE:-/userdata/ums_shared.img}"
		echo "export UMS_SIZE=${RK_USB_UMS_SIZE:-256M}"
		echo "export UMS_FSTYPE=${RK_USB_UMS_FSTYPE:-vfat}"
		echo "export UMS_MOUNT=$([ -z "$RK_USB_UMS_MOUNT" ] || echo 1)"
		echo "export UMS_MOUNTPOINT=${RK_USB_UMS_MOUNTPOINT:-/mnt/ums}"
		echo "export UMS_RO=$([ -z "$RK_USB_UMS_RO" ] || echo 1)"
	} >> "$TARGET_DIR/etc/profile.d/usbdevice.sh"
}

usb_funcs()
{
	{
		echo "${RK_USB_ADBD:+adb}"
		echo "${RK_USB_MTP:+mtp}"
		echo "${RK_USB_ACM:+acm}"
		echo "${RK_USB_NTB:+ntb}"
		echo "${RK_USB_UVC:+uvc}"
		echo "${RK_USB_UAC1:+uac1}"
		echo "${RK_USB_UAC2:+uac2}"
		echo "${RK_USB_HID:+hid}"
		echo "${RK_USB_RNDIS:+rndis}"
		echo "${RK_USB_UMS:+ums}"
		echo "$RK_USB_EXTRA"
	} | xargs
}

[ -z "$RK_USB_DISABLED" ] || exit 0

if [ "$RK_USB_DEFAULT" -a "$POST_OS" = buildroot ]; then
	echo -e "\e[33mKeep original USB gadget for buildroot by default\e[0m"
	exit 0
fi

cd "$SDK_DIR"

mkdir -p "$TARGET_DIR/etc" "$TARGET_DIR/lib" \
	"$TARGET_DIR/usr/bin" "$TARGET_DIR/usr/lib"

find "$TARGET_DIR/etc" "$TARGET_DIR/lib" "$TARGET_DIR/usr/bin" \
	"$TARGET_DIR/usr/lib" -name "*usbdevice*" -print0 | xargs -0 rm -rf
find "$TARGET_DIR/etc" -name ".usb_config" -print0 | xargs -0 rm -rf

echo "USB gadget functions: $(usb_funcs)"
mkdir -p "$TARGET_DIR/etc/profile.d"
echo "export USB_FUNCS=\"$(usb_funcs)\"" > "$TARGET_DIR/etc/profile.d/usbdevice.sh"

install_adbd
install_ums

mkdir -p "$TARGET_DIR/lib/udev/rules.d"
install -m 0644 external/rkscript/61-usbdevice.rules \
	"$TARGET_DIR/lib/udev/rules.d/"

install -m 0755 external/rkscript/usbdevice "$TARGET_DIR/usr/bin/"

echo "Installing USB services..."

if [ "$POST_INIT_SYSTEMD" ]; then
	install -m 0755 external/rkscript/usbdevice.service \
		"$TARGET_DIR/lib/systemd/system/"
	mkdir -p "$TARGET_DIR/etc/systemd/system/sysinit.target.wants"
	ln -sf /lib/systemd/system/usbdevice.service \
		"$TARGET_DIR/etc/systemd/system/sysinit.target.wants/"
fi

if [ "$POST_INIT_SYSV" ]; then
	install -m 0755 external/rkscript/S50usbdevice \
		"$TARGET_DIR/etc/init.d/usbdevice.sh"
	for level in 5 4 3 2; do
		mkdir -p "$TARGET_DIR/etc/rc${level}.d"
		ln -sf ../init.d/usbdevice.sh \
			"$TARGET_DIR/etc/rc${level}.d/S70usbdevice.sh"
	done
	for level in 0 1 6; do
		mkdir -p "$TARGET_DIR/etc/rc${level}.d"
		ln -sf ../init.d/usbdevice.sh \
			"$TARGET_DIR/etc/rc${level}.d/K30usbdevice.sh"
	done
fi

if [ "$POST_INIT_BUSYBOX" ]; then
	install -m 0755 external/rkscript/S50usbdevice "$TARGET_DIR/etc/init.d/"
fi

mkdir -p "$TARGET_DIR/etc/usbdevice.d"
for hook in $RK_USB_HOOKS; do
	[ -r "$hook" ] || continue
	echo "Installing USB hook: $hook"
	install -m 0644 "$hook" "$TARGET_DIR/etc/usbdevice.d/"
done
