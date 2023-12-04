################################################################################
#
# unscd
#
################################################################################

UNSCD_VERSION = 0.54
UNSCD_SOURCE = nscd-$(UNSCD_VERSION).c
UNSCD_SITE = http://busybox.net/~vda/unscd
UNSCD_LICENSE = GPL-2.0
UNSCD_LICENSE_FILES = $(UNSCD_SOURCE)

define UNSCD_EXTRACT_CMDS
	cp $(UNSCD_DL_DIR)/$($(PKG)_SOURCE) $(@D)/
endef

define UNSCD_BUILD_CMDS
	cd $(@D); \
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o nscd $(UNSCD_SOURCE)
endef

define UNSCD_INSTALL_TARGET_CMDS
	$(INSTALL) -m 755 -D $(@D)/nscd $(TARGET_DIR)/usr/sbin/nscd
	$(INSTALL) -m 600 -D package/unscd/nscd.conf $(TARGET_DIR)/etc/nscd.conf
endef

ifeq ($(BR2_PACKAGE_UNSCD_HOSTS_CACHE),y)
define UNSCD_ENABLE_HOSTS_CACHE
	sed -i "s/\(enable-cache.*\)no/\1yes/" $(TARGET_DIR)/etc/nscd.conf
endef
UNSCD_POST_INSTALL_TARGET_HOOKS += UNSCD_ENABLE_HOSTS_CACHE
endif

define UNSCD_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D package/unscd/S46unscd \
		$(TARGET_DIR)/etc/init.d/S46unscd
endef

define UNSCD_USERS
	unscd -1 unscd -1 * - - - unscd user
endef

$(eval $(generic-package))
