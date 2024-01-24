#!/bin/bash -e

install -d "${ROOTFS_DIR}/etc/systemd/system/getty@tty1.service.d"
install -v -m 0644 files/noclear.conf "${ROOTFS_DIR}/etc/systemd/system/getty@tty1.service.d/noclear.conf"
install -v -m 0644 files/serial-getty@.service "${ROOTFS_DIR}/etc/systemd/system/"
install -v -m 0644 files/fstab "${ROOTFS_DIR}/etc/fstab"

on_chroot << EOF
if ! id -u ${FIRST_USER_NAME} >/dev/null 2>&1; then
	adduser --shell /bin/bash --disabled-password --gecos "" ${FIRST_USER_NAME}
fi
echo "${FIRST_USER_NAME}:${FIRST_USER_PASS}" | chpasswd
echo "root:root" | chpasswd
EOF

chmod 777 ${ROOTFS_DIR}/etc/sudoers
echo "${FIRST_USER_NAME}    ALL=(ALL:ALL) ALL" >> ${ROOTFS_DIR}/etc/sudoers
chmod 440 ${ROOTFS_DIR}/etc/sudoers

install -d				"${ROOTFS_DIR}/etc/systemd/system/rc-local.service.d"
install -v -m 0644 files/ttyoutput.conf	"${ROOTFS_DIR}/etc/systemd/system/rc-local.service.d/"

install -v -m 0644 files/50raspi		"${ROOTFS_DIR}/etc/apt/apt.conf.d/"

install -v -m 0644 files/console-setup   	"${ROOTFS_DIR}/etc/default/"

install -v -m 0755 files/rc.local		"${ROOTFS_DIR}/etc/"

on_chroot <<EOF
for GRP in input spi i2c gpio; do
	groupadd -f -r "\$GRP"
done
for GRP in adm dialout cdrom audio users sudo video games plugdev input gpio spi i2c netdev render; do
  adduser $FIRST_USER_NAME \$GRP
done
EOF

on_chroot << EOF
setupcon --force --save-only -v
EOF

on_chroot << EOF
usermod --pass='*' root
EOF
