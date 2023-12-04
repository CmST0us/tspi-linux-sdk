################################################################################
#
# deviceio_release
#
################################################################################
DEVICEIO_RELEASE_SITE = $(TOPDIR)/../external/deviceio_release
DEVICEIO_RELEASE_SITE_METHOD = local
DEVICEIO_RELEASE_INSTALL_STAGING = YES
DEVICEIO_RELEASE_DEPENDENCIES += wpa_supplicant alsa-lib
BT_TTY_DEV = $(call qstrip,$(BR2_PACKAGE_RKWIFIBT_BTUART))
ifeq ($(call qstrip,$(BR2_ARCH)), arm)
        DEVICEIOARCH = lib32
else ifeq ($(call qstrip, $(BR2_ARCH)), aarch64)
        DEVICEIOARCH = lib64
endif

ifeq 	($(call qstrip,$(BR2_PACKAGE_RKWIFIBT_VENDOR)), ROCKCHIP)
	LIBDEVICEIOSO = bluez/libDeviceIo.so
	DEVICEIO_RELEASE_DEPENDENCIES += readline bluez5_utils libglib2 
else ifeq ($(call qstrip,$(BR2_PACKAGE_RKWIFIBT_VENDOR)), REALTEK)
	LIBDEVICEIOSO = bluez/libDeviceIo.so
	DEVICEIO_RELEASE_DEPENDENCIES += readline bluez5_utils libglib2 bluez-alsa
else ifeq ($(call qstrip,$(BR2_PACKAGE_RKWIFIBT_VENDOR)), BROADCOM)
	LIBDEVICEIOSO = broadcom/libDeviceIo.so
else ifeq ($(call qstrip,$(BR2_PACKAGE_RKWIFIBT_VENDOR)), CYPRESS)
	LIBDEVICEIOSO = cypress/libDeviceIo.so
else
	LIBDEVICEIOSO = fake/libDeviceIo.so
endif

ifeq ($(BR2_PACKAGE_RV1126_RV1109),y)
        PLATFORMPATH = rv1126_rv1109
else
        PLATFORMPATH = common
endif

define DEVICEIO_RELEASE_INSTALL_COMMON
	$(INSTALL) -D -m 0755 $(STAGING_DIR)/usr/bin/deviceio_test $(TARGET_DIR)/usr/bin/deviceio_test
endef

define DEVICEIO_RELEASE_INSTALL_TARGET_CMDS
	$(DEVICEIO_RELEASE_INSTALL_COMMON)
endef

define DEVICEIO_PRE_BUILD_HOOK
	$(INSTALL) -D -m 0755 $(@D)/DeviceIO/$(PLATFORMPATH)/$(DEVICEIOARCH)/$(LIBDEVICEIOSO) $(TARGET_DIR)/usr/lib/libDeviceIo.so
	$(INSTALL) -D -m 0755 $(@D)/DeviceIO/$(PLATFORMPATH)/$(DEVICEIOARCH)/$(LIBDEVICEIOSO) $(STAGING_DIR)/usr/lib/libDeviceIo.so
endef

DEVICEIO_RELEASE_PRE_BUILD_HOOKS += DEVICEIO_PRE_BUILD_HOOK

DEVICEIO_RELEASE_CONF_OPTS += -DCMAKE_INSTALL_STAGING=$(STAGING_DIR)

$(eval $(cmake-package))
