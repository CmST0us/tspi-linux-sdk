################################################################################
#
# lvgl
#
################################################################################

LVGL_VERSION = 8.2.0
LVGL_SOURCE = v$(LVGL_VERSION).tar.gz
LVGL_SITE = https://github.com/lvgl/lvgl/archive/refs/tags
LVGL_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_RK_OEM), y)
LVGL_INSTALL_TARGET_OPTS = DESTDIR=$(BR2_PACKAGE_RK_OEM_INSTALL_TARGET_DIR) install/fast
LVGL_DEPENDENCIES += rk_oem
endif

$(eval $(cmake-package))