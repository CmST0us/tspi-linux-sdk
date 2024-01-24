#!/bin/bash -e

echo "${TARGET_HOSTNAME}" > "${ROOTFS_DIR}/etc/hostname"
echo "127.0.1.1		${TARGET_HOSTNAME}" >> "${ROOTFS_DIR}/etc/hosts"

if [ -n "${PUBKEY_SSH_FIRST_USER}" ]; then
	install -v -m 0700 -o 1000 -g 1000 -d "${ROOTFS_DIR}"/home/"${FIRST_USER_NAME}"/.ssh
	echo "${PUBKEY_SSH_FIRST_USER}" >"${ROOTFS_DIR}"/home/"${FIRST_USER_NAME}"/.ssh/authorized_keys
	chown 1000:1000 "${ROOTFS_DIR}"/home/"${FIRST_USER_NAME}"/.ssh/authorized_keys
	chmod 0600 "${ROOTFS_DIR}"/home/"${FIRST_USER_NAME}"/.ssh/authorized_keys
fi

if [ "${PUBKEY_ONLY_SSH}" = "1" ]; then
	sed -i -Ee 's/^#?[[:blank:]]*PubkeyAuthentication[[:blank:]]*no[[:blank:]]*$/PubkeyAuthentication yes/
s/^#?[[:blank:]]*PasswordAuthentication[[:blank:]]*yes[[:blank:]]*$/PasswordAuthentication no/' "${ROOTFS_DIR}"/etc/ssh/sshd_config
fi

on_chroot << EOF
systemctl disable NetworkManager-wait-online.service
if [ "${ENABLE_SSH}" == "1" ]; then
	systemctl enable ssh
else
	systemctl disable ssh
fi
EOF
