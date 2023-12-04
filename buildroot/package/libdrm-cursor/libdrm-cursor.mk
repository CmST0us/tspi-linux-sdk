################################################################################
#
# libdrm-cursor
#
################################################################################

LIBDRM_CURSOR_SITE = https://github.com/JeffyCN/drm-cursor.git
LIBDRM_CURSOR_VERSION = 1.3.0
LIBDRM_CURSOR_SITE_METHOD = git
LIBDRM_CURSOR_AUTORECONF = YES

LIBDRM_CURSOR_LICENSE = LGPL-2.1
LIBDRM_CURSOR_LICENSE_FILES = COPYING

LIBDRM_CURSOR_DEPENDENCIES = libdrm

define LIBDRM_CURSOR_PRELOAD
	cd $(TARGET_DIR)/etc/; \
		grep "libdrm-cursor.so" ld.so.preload 2>/dev/null || \
		echo "/usr/lib/libdrm-cursor.so" >> ld.so.preload
endef
LIBDRM_CURSOR_POST_INSTALL_TARGET_HOOKS += LIBDRM_CURSOR_PRELOAD

$(eval $(meson-package))
