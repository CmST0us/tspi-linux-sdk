BROADCOM_BSA_SITE = $(TOPDIR)/../external/broadcom_bsa
BROADCOM_BSA_SITE_METHOD = local
BROADCOM_BSA_INSTALL_STAGING = YES

BT_TTY_DEV = $(call qstrip,$(BR2_PACKAGE_RKWIFIBT_BTUART))
BT_FW_DIR = system

ifeq ($(BR2_PACKAGE_BROADCOM_BSA)$(BR2_PACKAGE_CYPRESS_BSA),yy)
$(error "You can only choose one type of BSA module (Broadcom or Cypress).")
endif

ifeq ($(call qstrip,$(BR2_ARCH)),arm)
BROADCOM_BSA_BUILD_TYPE = arm
endif
ifeq ($(call qstrip,$(BR2_ARCH)),aarch64)
BROADCOM_BSA_BUILD_TYPE = arm64
endif

BTFIRMWARE = $(BR2_PACKAGE_RKWIFIBT_BT_FW)

define BROADCOM_BSA_INSTALL_STAGING_CMDS
        $(INSTALL) -D -m 644 $(@D)/libbsa/build/$(BROADCOM_BSA_BUILD_TYPE)/sharedlib/libbsa.so \
                $(STAGING_DIR)/usr/lib/libbsa.so
endef

define BROADCOM_BSA_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/server/$(BROADCOM_BSA_BUILD_TYPE)/bsa_server \
		$(TARGET_DIR)/usr/bin/bsa_server
	$(INSTALL) -D -m 644 $(@D)/libbsa/build/$(BROADCOM_BSA_BUILD_TYPE)/sharedlib/libbsa.so \
		$(TARGET_DIR)/usr/lib/libbsa.so

	mkdir -p $(TARGET_DIR)/etc/bsa_file
	$(INSTALL) -D -m 644 $(TOPDIR)/../external/broadcom_bsa/test_files/av/8k8bpsMono.wav $(TARGET_DIR)/etc/bsa_file/
	$(INSTALL) -D -m 644 $(TOPDIR)/../external/broadcom_bsa/test_files/av/8k16bpsStereo.wav $(TARGET_DIR)/etc/bsa_file/
	$(INSTALL) -D -m 755 $(TOPDIR)/../external/broadcom_bsa/bsa_server.sh $(TARGET_DIR)/usr/bin/
	sed -i 's/BTFIRMWARE_PATH/\/$(BT_FW_DIR)\/etc\/firmware\//g' $(TARGET_DIR)/usr/bin/bsa_server.sh
	sed -i 's/BT_TTY_DEV/\/dev\/$(BT_TTY_DEV)/g' $(TARGET_DIR)/usr/bin/bsa_server.sh
endef

$(eval $(generic-package))
