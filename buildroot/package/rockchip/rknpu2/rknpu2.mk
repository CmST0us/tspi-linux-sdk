################################################################################
#
# rknpu2
#
################################################################################
RKNPU2_VERSION = 1.0.0
RKNPU2_SITE_METHOD = local
RKNPU2_SITE = $(TOPDIR)/../external/rknpu2
RKNPU2_INSTALL_STAGING = YES

RKNPU2_LICENSE = ROCKCHIP
RKNPU2_LICENSE_FILES = LICENSE

RKNPU2_ARCH = $(call qstrip,$(BR2_PACKAGE_RKNPU2_ARCH))
RKNPU2_SOC = $(call qstrip,$(BR2_PACKAGE_RKNPU2_SOC))

define RKNPU2_INSTALL_TARGET_CMDS
	cp -r $(@D)/runtime/$(RKNPU2_SOC)/Linux/rknn_server/$(RKNPU2_ARCH)/* \
		$(TARGET_DIR)/
	cp -r $(@D)/runtime/$(RKNPU2_SOC)/Linux/librknn_api/$(RKNPU2_ARCH)/* \
		$(TARGET_DIR)/usr/lib/
endef

define RKNPU2_INSTALL_STAGING_CMDS
	cp -r $(@D)/runtime/$(RKNPU2_SOC)/Linux/librknn_api/$(RKNPU2_ARCH)/* \
		$(STAGING_DIR)/usr/lib/
	cp -rT $(@D)/runtime/$(RKNPU2_SOC)/Linux/librknn_api/include \
		$(STAGING_DIR)/usr/include/rknn
endef

ifeq ($(BR2_PACKAGE_RKNPU2_EXAMPLE),)
$(eval $(generic-package))
else
RKNPU2_SUBDIR = examples/rknn_common_test
RKNPU2_CONF_OPTS += -DTARGET_SOC=$(call LOWERCASE,$(RKNPU2_SOC))

define RKNPU2_INSTALL_TARGET_EXAMPLE
	cp -r $(@D)/runtime/$(RKNPU2_SOC)/Linux/librknn_api/$(RKNPU2_ARCH)/* \
		$(STAGING_DIR)/usr/lib/
	cp -rT $(@D)/runtime/$(RKNPU2_SOC)/Linux/librknn_api/include \
		$(STAGING_DIR)/usr/include/rknn

	cp $(@D)/$(RKNPU2_SUBDIR)/rknn_common_test $(TARGET_DIR)/usr/bin/
	cp -r $(@D)/$(RKNPU2_SUBDIR)/model $(TARGET_DIR)/usr/share/
endef
RKNPU2_POST_INSTALL_TARGET_HOOKS += RKNPU2_INSTALL_TARGET_EXAMPLE
$(eval $(cmake-package))
endif
