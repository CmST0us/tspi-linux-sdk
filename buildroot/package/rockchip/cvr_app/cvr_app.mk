CVR_APP_SITE = $(TOPDIR)/../app/cvr_app
CVR_APP_SITE_METHOD = local

# add dependencies
CVR_APP_DEPENDENCIES = rkfsmk camera_engine_rkaiq rkadk rockchip-rga lvgl

CVR_APP_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_RK_OEM), y)
CVR_APP_INSTALL_TARGET_OPTS = DESTDIR=$(BR2_PACKAGE_RK_OEM_INSTALL_TARGET_DIR) install/fast
CVR_APP_DEPENDENCIES += rk_oem
endif

$(eval $(cmake-package))
