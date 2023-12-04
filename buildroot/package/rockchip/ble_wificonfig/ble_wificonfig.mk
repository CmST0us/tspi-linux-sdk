# Rockchip's ble wificonfig service base on bluez5 gatt-service

BLE_WIFICONFIG_SITE = $(TOPDIR)/../external/ble_wificonfig
BLE_WIFICONFIG_SITE_METHOD = local
BLE_WIFICONFIG_INSTALL_STAGING = YES
$(eval $(cmake-package))


