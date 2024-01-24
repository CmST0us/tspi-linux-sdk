################################################################################
#
# ntfs-3g
#
################################################################################

NTFS_3G_VERSION = 2022.5.17
NTFS_3G_SOURCE = ntfs-3g_ntfsprogs-$(NTFS_3G_VERSION).tgz
NTFS_3G_SITE = http://tuxera.com/opensource
NTFS_3G_CONF_OPTS = --disable-ldconfig
NTFS_3G_INSTALL_STAGING = YES
NTFS_3G_DEPENDENCIES = host-pkgconf
NTFS_3G_LICENSE = GPL-2.0+, LGPL-2.0+
NTFS_3G_LICENSE_FILES = COPYING COPYING.LIB
NTFS_3G_CPE_ID_VENDOR = tuxera

HOST_NTFS_3G_CONF_OPTS += --prefix=/ --disable-ldconfig
HOST_NTFS_3G_INSTALL_OPTS += DESTDIR=$(HOST_DIR) install

ifeq ($(BR2_PACKAGE_LIBFUSE),y)
NTFS_3G_CONF_OPTS += --with-fuse=external
NTFS_3G_DEPENDENCIES += libfuse
endif

ifeq ($(BR2_PACKAGE_UTIL_LINUX_LIBUUID),y)
NTFS_3G_DEPENDENCIES += util-linux
endif

ifeq ($(BR2_PACKAGE_NTFS_3G_ENCRYPTED),y)
NTFS_3G_CONF_ENV += LIBGCRYPT_CONFIG=$(STAGING_DIR)/usr/bin/libgcrypt-config
NTFS_3G_CONF_OPTS += --enable-crypto
NTFS_3G_DEPENDENCIES += gnutls libgcrypt
endif

ifneq ($(BR2_PACKAGE_NTFS_3G_NTFSPROGS),y)
NTFS_3G_CONF_OPTS += --disable-ntfsprogs
endif

# Create symlink to mount.ntfs
define NTFS_3G_INSTALL_SYMLINK
	ln -sf mount.ntfs-3g $(TARGET_DIR)/sbin/mount.ntfs
endef
NTFS_3G_POST_INSTALL_TARGET_HOOKS += NTFS_3G_INSTALL_SYMLINK

# Create wrapper for fsck helper
define NTFS_3G_INSTALL_WRAPPER
	FSCK_NTFS=$(TARGET_DIR)/sbin/fsck.ntfs; \
	if [ -f $(TARGET_DIR)/usr/bin/ntfsfix ];then \
		echo "#!/bin/sh" > $(TARGET_DIR)/sbin/fsck.ntfs; \
		echo 'ntfsfix $$(echo $$@ |xargs -n 1|grep "^[^-]")' \
			>> $(TARGET_DIR)/sbin/fsck.ntfs; \
		chmod 755 $(TARGET_DIR)/sbin/fsck.ntfs; \
	fi
endef
NTFS_3G_POST_INSTALL_TARGET_HOOKS += NTFS_3G_INSTALL_WRAPPER

$(eval $(autotools-package))
$(eval $(host-autotools-package))
