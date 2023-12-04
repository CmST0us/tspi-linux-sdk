################################################################################
#
# Rockchip CAMERA ENGINE CIFISP For Linux
#
################################################################################

CAMERA_ENGINE_CIFISP_VERSION = 1.0
CAMERA_ENGINE_CIFISP_SITE = $(TOPDIR)/../external/camera_engine_cifisp
CAMERA_ENGINE_CIFISP_SITE_METHOD = local

CAMERA_ENGINE_CIFISP_LICENSE = Apache V2.0
CAMERA_ENGINE_CIFISP_LICENSE_FILES = NOTICE

CAMERA_ENGINE_CIFISP_MAKE_OPTS = \
	TARGET_GCC="$(TARGET_CC)" \
	TARGET_GPP="$(TARGET_CXX)" \
	TARGET_AR="$(TARGET_AR)" \
	TARGET_LD="$(TARGET_LD)" \

CAMERA_ENGINE_RKISP_CONF_OPTS = \
	ARCH=$(BR2_ARCH)

define CAMERA_ENGINE_CIFISP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
		$(CAMERA_ENGINE_CIFISP_MAKE_OPTS) \
		$(CAMERA_ENGINE_RKISP_CONF_OPTS)
endef

define CAMERA_ENGINE_CIFISP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m  644 $(@D)/build/lib/libcam_ia.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m  644 $(@D)/build/lib/libcam_engine_cifisp.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(autotools-package))
