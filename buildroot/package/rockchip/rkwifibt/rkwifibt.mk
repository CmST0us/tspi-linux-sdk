################################################################################
#
# rkwifibt
#
################################################################################

RKWIFIBT_VERSION = 1.0.0
RKWIFIBT_SITE_METHOD = local
RKWIFIBT_SITE = $(TOPDIR)/../external/rkwifibt
RKWIFIBT_LICENSE = Apache V2.0
RKWIFIBT_LICENSE_FILES = NOTICE

RKWIFIBT_DEPENDENCIES = wpa_supplicant

BT_TTY_DEV = $(call qstrip,$(BR2_PACKAGE_RKWIFIBT_BTUART))
SXLOAD_WIFI = "S36load_wifi_modules"
FIRMWARE_DIR = system

ifeq ($(call qstrip,$(BR2_ARCH)),aarch64)
RKWIFIBT_ARCH=arm64
else ifeq ($(call qstrip,$(BR2_ARCH)),arm)
RKWIFIBT_ARCH=arm
endif

BT_DRIVER_ARCH = $(shell grep -o "arm[^ ]*" $(TOPDIR)/../kernel/.config)
ifeq ($(call qstrip,$(BT_DRIVER_ARCH)),arm64)
BT_DRIVER_ARCH = arm64
else
BT_DRIVER_ARCH = arm
endif

ifeq ($(BR2_PACKAGE_RV1126_RV1109),y)
SXLOAD_WIFI = "S36load_rv1109_wifi_modules"
RK_WIFI_CHIP_NAME1 = AP6256
RK_WIFI_CHIP_NAME2 = AP6255
endif

ifeq (y,$(BR2_PACKAGE_RKWIFIBT_AMPAKALL))
SXLOAD_WIFI = "S36load_ampakall_wifi_modules"
endif

define RKWIFIBT_INSTALL_COMMON
    mkdir -p $(TARGET_DIR)/lib/firmware $(TARGET_DIR)/usr/lib/modules $(TARGET_DIR)/$(FIRMWARE_DIR)/etc/firmware $(TARGET_DIR)/lib/firmware/rtlbt
    $(INSTALL) -D -m 0755 $(@D)/wpa_supplicant.conf $(TARGET_DIR)/etc/
    $(INSTALL) -D -m 0755 $(@D)/dnsmasq.conf $(TARGET_DIR)/etc/
    $(INSTALL) -D -m 0755 $(@D)/wifi_start.sh $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/wifi_ap6xxx_rftest.sh $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/src/rk_wifi_init $(TARGET_DIR)/usr/bin/
    $(SED) 's/WIFI_KO/\/$(FIRMWARE_DIR)\/lib\/modules\/$(BR2_PACKAGE_RKWIFIBT_WIFI_KO)/g' $(@D)/$(SXLOAD_WIFI)
    $(SED) 's/BT_TTY_DEV/\/dev\/$(BT_TTY_DEV)/g' $(@D)/$(SXLOAD_WIFI)
    -$(INSTALL) -D -m 0755 $(@D)/$(SXLOAD_WIFI) $(TARGET_DIR)/etc/init.d/
endef

define RKWIFIBT_TB_INSTALL
    mkdir -p $(TARGET_DIR)/$(FIRMWARE_DIR)/etc/firmware
    $(INSTALL) -D -m 0755 $(@D)/wpa_supplicant.conf $(TARGET_DIR)/etc/
    $(INSTALL) -D -m 0644 $(@D)/firmware/broadcom/$(BR2_PACKAGE_RKWIFIBT_CHIPNAME)/wifi/* $(TARGET_DIR)/$(FIRMWARE_DIR)/etc/firmware/
    $(INSTALL) -D -m 0755 $(@D)/tb_start_wifi.sh $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/brcm_tools/dhd_priv $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/src/CY_WL_API/keepalive $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/bin/$(RKWIFIBT_ARCH)/* $(TARGET_DIR)/usr/bin/

    $(INSTALL) -D -m 0644 $(TOPDIR)/../kernel/drivers/net/wireless/rockchip_wlan/rkwifi/rk_wifi_config.ko $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/
    $(INSTALL) -D -m 0644 $(TOPDIR)/../kernel/net/rfkill/rfkill.ko $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/
    $(INSTALL) -D -m 0644 $(TOPDIR)/../kernel/net/rfkill/rfkill-rk.ko $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/
    $(INSTALL) -D -m 0644 $(TOPDIR)/../kernel/net/wireless/cfg80211.ko $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/
    $(INSTALL) -D -m 0644 $(TOPDIR)/../kernel/net/mac80211/mac80211.ko $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/
    -$(TARGET_STRIP) $(STRIP_STRIP_DEBUG) $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/*.ko
endef

define RKWIFIBT_BROADCOM_INSTALL
    $(INSTALL) -D -m 0644 $(@D)/firmware/broadcom/$(BR2_PACKAGE_RKWIFIBT_CHIPNAME)/wifi/* $(TARGET_DIR)/$(FIRMWARE_DIR)/etc/firmware/
    -$(INSTALL) -D -m 0644 $(@D)/firmware/broadcom/$(RK_WIFI_CHIP_NAME1)/wifi/* $(TARGET_DIR)/$(FIRMWARE_DIR)/etc/firmware/
    -$(INSTALL) -D -m 0644 $(@D)/firmware/broadcom/$(RK_WIFI_CHIP_NAME2)/wifi/* $(TARGET_DIR)/$(FIRMWARE_DIR)/etc/firmware/
    $(INSTALL) -D -m 0755 $(@D)/brcm_tools/brcm_patchram_plus1 $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/brcm_tools/dhd_priv $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/bin/$(RKWIFIBT_ARCH)/* $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0644 $(@D)/firmware/broadcom/$(BR2_PACKAGE_RKWIFIBT_CHIPNAME)/bt/* $(TARGET_DIR)/$(FIRMWARE_DIR)/etc/firmware/
    $(INSTALL) -D -m 0755 $(@D)/bt_load_broadcom_firmware $(TARGET_DIR)/usr/bin/
    $(SED) 's/BTFIRMWARE_PATH/\/$(FIRMWARE_DIR)\/etc\/firmware\/$(BR2_PACKAGE_RKWIFIBT_BT_FW)/g' $(TARGET_DIR)/usr/bin/bt_load_broadcom_firmware
    $(SED) 's/BT_TTY_DEV/\/dev\/$(BT_TTY_DEV)/g' $(TARGET_DIR)/usr/bin/bt_load_broadcom_firmware
    $(INSTALL) -D -m 0755 $(TARGET_DIR)/usr/bin/bt_load_broadcom_firmware $(TARGET_DIR)/usr/bin/bt_pcba_test
    $(INSTALL) -D -m 0755 $(TARGET_DIR)/usr/bin/bt_load_broadcom_firmware $(TARGET_DIR)/usr/bin/bt_init.sh
endef

define RKWIFIBT_REALTEK_WIFI_INSTALL
    $(INSTALL) -D -m 0755 $(@D)/bin/$(RKWIFIBT_ARCH)/rtwpriv $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/$(SXLOAD_WIFI) $(TARGET_DIR)/etc/init.d/
endef

define RKWIFIBT_REALTEK_BT_INSTALL
    $(INSTALL) -D -m 0755 $(@D)/realtek/rtk_hciattach/rtk_hciattach $(TARGET_DIR)/usr/bin/rtk_hciattach
    $(INSTALL) -D -m 0755 $(@D)/bin/$(RKWIFIBT_ARCH)/* $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0644 $(@D)/realtek/$(BR2_PACKAGE_RKWIFIBT_CHIPNAME)/* $(TARGET_DIR)/lib/firmware/rtlbt/
    -$(INSTALL) -D -m 0644 $(@D)/realtek/$(BR2_PACKAGE_RKWIFIBT_CHIPNAME)/mp_* $(TARGET_DIR)/lib/firmware/rtlbt/
    -$(INSTALL) -D -m 0644 $(@D)/realtek/$(BR2_PACKAGE_RKWIFIBT_CHIPNAME)/mp_* $(TARGET_DIR)/lib/firmware/
    $(INSTALL) -D -m 0755 $(@D)/bt_realtek* $(TARGET_DIR)/usr/bin/
    -$(INSTALL) -D -m 0644 $(@D)/realtek/bluetooth_uart_driver/hci_uart.ko $(TARGET_DIR)/usr/lib/modules/hci_uart.ko
    $(INSTALL) -D -m 0755 $(@D)/bt_load_rtk_firmware $(TARGET_DIR)/usr/bin/
    $(SED) 's/BT_TTY_DEV/\/dev\/$(BT_TTY_DEV)/g' $(TARGET_DIR)/usr/bin/bt_load_rtk_firmware
    $(INSTALL) -D -m 0755 $(TARGET_DIR)/usr/bin/bt_load_rtk_firmware $(TARGET_DIR)/usr/bin/bt_pcba_test
    $(INSTALL) -D -m 0755 $(TARGET_DIR)/usr/bin/bt_load_rtk_firmware $(TARGET_DIR)/usr/bin/bt_init.sh
endef

ifeq ($(BR2_PACKAGE_RKWIFIBT_CHIPNAME), "RK915")
define RKWIFIBT_ROCKCHIP_INSTALL
    $(SED) 's/BT_TTY_DEV/\/dev\/$(BT_TTY_DEV)/g' $(@D)/$(SXLOAD_WIFI)
    $(SED) 's/WIFI_KO/\/$(FIRMWARE_DIR)\/lib\/modules\/$(BR2_PACKAGE_RKWIFIBT_WIFI_KO)/g' $(@D)/$(SXLOAD_WIFI)
    $(INSTALL) -D -m 0755 $(@D)/$(SXLOAD_WIFI) $(TARGET_DIR)/etc/init.d/
    $(INSTALL) -D -m 0644 $(@D)/firmware/rockchip/WIFI_FIRMWARE/rk915* $(TARGET_DIR)/system/etc/firmware/
endef
else
define RKWIFIBT_ROCKCHIP_INSTALL
    $(INSTALL) -D -m 0644 $(@D)/firmware/rockchip/WIFI_FIRMWARE/rk912* $(TARGET_DIR)/lib/firmware/
    $(INSTALL) -D -m 0755 $(@D)/S36load_wifi_rk912_modules $(TARGET_DIR)/etc/init.d/
endef
endif

define RKWIFIBT_BUILD_CMDS
    ln -sf $(FIRMWARE_DIR) $(TARGET_DIR)/vendor
    mkdir -p $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/
    -$(TOPDIR)/../build.sh modules
    find $(TOPDIR)/../kernel/drivers/net/wireless/rockchip_wlan/* -name $(BR2_PACKAGE_RKWIFIBT_WIFI_KO) | xargs -n1 -i cp {} $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/
    -$(TARGET_STRIP) $(STRIP_STRIP_DEBUG) $(TARGET_DIR)/$(FIRMWARE_DIR)/lib/modules/*.ko
    $(TARGET_CC) -o $(@D)/brcm_tools/brcm_patchram_plus1 $(@D)/brcm_tools/brcm_patchram_plus1.c
    $(TARGET_CC) -o $(@D)/brcm_tools/dhd_priv $(@D)/brcm_tools/dhd_priv.c
    $(TARGET_CC) -o $(@D)/src/rk_wifi_init $(@D)/src/rk_wifi_init.c
    $(MAKE) -C $(@D)/realtek/rtk_hciattach/ CC=$(TARGET_CC)
    $(MAKE) -C $(@D)/src/CY_WL_API/ CC=$(TARGET_CC)
    -$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(TOPDIR)/../kernel/ M=$(@D)/realtek/bluetooth_uart_driver ARCH=$(BT_DRIVER_ARCH)
endef

ifneq ($(BR2_PACKAGE_THUNDERBOOT), y)

ifeq ($(BR2_PACKAGE_RKWIFIBT_VENDOR), "BROADCOM")
define RKWIFIBT_INSTALL_TARGET_CMDS
    $(RKWIFIBT_INSTALL_COMMON)
    $(RKWIFIBT_BROADCOM_INSTALL)
endef
endif

ifeq ($(BR2_PACKAGE_RKWIFIBT_VENDOR), "CYPRESS")
define RKWIFIBT_INSTALL_TARGET_CMDS
    $(RKWIFIBT_INSTALL_COMMON)
    $(RKWIFIBT_BROADCOM_INSTALL)
endef
endif

else

define RKWIFIBT_INSTALL_TARGET_CMDS
    $(RKWIFIBT_TB_INSTALL)
endef

endif #THUNDERBOOT

ifeq ($(BR2_PACKAGE_RKWIFIBT_VENDOR), "REALTEK")

ifeq ($(BR2_PACKAGE_RKWIFIBT_BT_EN), "ENABLE")
define RKWIFIBT_INSTALL_TARGET_CMDS
    $(RKWIFIBT_INSTALL_COMMON)
    $(RKWIFIBT_REALTEK_WIFI_INSTALL)
    $(RKWIFIBT_REALTEK_BT_INSTALL)
endef
else
define RKWIFIBT_INSTALL_TARGET_CMDS
    $(RKWIFIBT_INSTALL_COMMON)
    $(RKWIFIBT_REALTEK_WIFI_INSTALL)
endef
endif

endif

ifeq ($(BR2_PACKAGE_RKWIFIBT_VENDOR), "ROCKCHIP")
define RKWIFIBT_INSTALL_TARGET_CMDS
    $(RKWIFIBT_INSTALL_COMMON)
    $(RKWIFIBT_ROCKCHIP_INSTALL)
endef
endif

define RKWIFIBT_POST_INSTALL_TARGET_HOOKS_CMDS
	-rm -f $(@D)/$(SXLOAD_WIFI)
endef

RKWIFIBT_POST_INSTALL_TARGET_HOOKS += RKWIFIBT_POST_INSTALL_TARGET_HOOKS_CMDS

$(eval $(generic-package))
