#!/bin/bash -e

install -v -m 0644 -D $TOP_DIR/external/rkscript/61-partition-init.rules $ROOTFS_DIR/lib/udev/rules.d/

# sd card udev rule
install -v -m 0644 -D $TOP_DIR/external/rkscript/61-sd-cards-auto-mount.rules $ROOTFS_DIR/lib/udev/rules.d/

# usb disk
install -v -m 0644 -D files/automount.service $ROOTFS_DIR/etc/systemd/system/
install -v -m 0755 -D files/automount.sh $ROOTFS_DIR/usr/bin/

# adbd, ums, mtp
install -v -m 0644 -D $TOP_DIR/external/rkscript/usbdevice.service $ROOTFS_DIR/etc/systemd/system/
install -v -m 0755 -D $TOP_DIR/external/rkscript/S50usbdevice $ROOTFS_DIR/etc/init.d/
echo usb_adb_en > $ROOTFS_DIR/etc/init.d/.usb_config
# echo usb_ums_en >> $ROOTFS_DIR/etc/init.d/.usb_config
install -v -m 0644 -D $TOP_DIR/external/rkscript/61-usbdevice.rules $ROOTFS_DIR/lib/udev/rules.d/
install -v -m 0755 -D $TOP_DIR/external/rkscript/usbdevice $ROOTFS_DIR/usr/bin/

if [ "${ARCH}" == "armhf" ]; then
	install -v -m 0755 $TOP_DIR/debian/overlay-debug/usr/local/share/adb/adbd-32 "${ROOTFS_DIR}/usr/bin/adbd"
else
	install -v -m 0755 $TOP_DIR/debian/overlay-debug/usr/local/share/adb/adbd-64 "${ROOTFS_DIR}/usr/bin/adbd"
fi

# resize rootfs
install -v -m 0755 -D $TOP_DIR/external/rkscript/resize-helper $ROOTFS_DIR/usr/sbin/
install -v -m 0755 -D $TOP_DIR/external/rkscript/S22resize-disk $ROOTFS_DIR/etc/init.d/
install -v -m 0644 -D $TOP_DIR/external/rkscript/resize-disk.service $ROOTFS_DIR/etc/systemd/system/

on_chroot << EOF
	systemctl enable usbdevice
	systemctl enable automount
	systemctl enable resize-disk
EOF

# mount all partitions in /etc/fstab
# install -v -m 0755 -D $TOP_DIR/external/rkscript/S21mountall.sh $TARGET_DIR/etc/init.d/

