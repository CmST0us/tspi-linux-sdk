ifeq ($(BR2_PACKAGE_DUI),y)

DUI_SITE = $(TOPDIR)/../external/dui
DUI_SITE_METHOD = local
DUI_INSTALL_STAGING = YES

ifeq ($(call qstrip,$(BR2_ARCH)), arm)
DUI_BUILD_TYPE = arm
else ifeq ($(call qstrip, $(BR2_ARCH)), aarch64)
DUI_BUILD_TYPE = arm64
endif

DUI_CONF_OPTS += -DCPU_ARCH=$(BR2_ARCH) -DBUILD_TYPE=$(DUI_BUILD_TYPE)

DUI_DEPENDENCIES += deviceio opus libcurl

$(eval $(cmake-package))

endif
