################################################################################
#
# libarm-memhook
#
################################################################################

LIBARM_MEMHOOK_SITE = http://github.com/JeffyCN/arm-memhook
LIBARM_MEMHOOK_VERSION = master
LIBARM_MEMHOOK_SITE_METHOD = git

LIBARM_MEMHOOK_LICENSE = GPL-2.0+
LIBARM_MEMHOOK_LICENSE_FILES = COPYING
LIBARM_MEMHOOK_CPE_ID_VENDOR = gnu

LIBARM_MEMHOOK_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_LIBARM_MEMHOOK_PRELOAD),y)
define LIBARM_MEMHOOK_PRELOAD
	cd $(TARGET_DIR)/etc/; \
		grep "libarm-memhook.so" ld.so.preload 2>/dev/null || \
		echo "/usr/lib/libarm-memhook.so" >> ld.so.preload
endef
LIBARM_MEMHOOK_POST_INSTALL_TARGET_HOOKS += LIBARM_MEMHOOK_PRELOAD
endif

$(eval $(meson-package))
