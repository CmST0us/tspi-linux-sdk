RV1108_UVC_APP_SITE = $(TOPDIR)/../app/uvc_app
RV1108_UVC_APP_SITE_METHOD = local
RV1108_UVC_APP_INSTALL_STAGING = YES

# add dependencies
RV1108_UVC_APP_DEPENDENCIES = camerahal rkfb rockchip-rga rockchip-mpp librkuvc

RV1108_UVC_APP_CONF_OPTS += -DISP_FMT=HAL_FRMAE_FMT_NV12
RV1108_UVC_APP_CONF_OPTS += -DCIF_FMT=HAL_FRMAE_FMT_NV12

ifeq ($(RK_UVC_USE_SL_MODULE),y)
    RV1108_UVC_APP_CONF_OPTS += -DCIF_FMT=HAL_FRMAE_FMT_YUYV
endif

define RV1108_UVC_APP_INSTALL_INIT_SYSV
    $(INSTALL) -m 0755 -D package/rockchip/rv1108_package/app/rv1108_uvc_app/S99_rv1108_uvc_app $(TARGET_DIR)/etc/init.d/S99_rv1108_uvc_app
endef

$(eval $(cmake-package))
