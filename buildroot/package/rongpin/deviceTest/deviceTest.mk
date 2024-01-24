################################################################################
# 
# testApp
#
################################################################################

DEVICETEST_VERSION = 1.0
DEVICETEST_SITE = ${TOPDIR}/../app/DeviceTest
DEVICETEST_SITE_METHOD = local

DEVICETEST_LICENSE = Rongpin
DEVICETEST_LICENSE_FILES = NOTICE
DEVICETEST_DEPENDENCIES = qt5multimedia qt5serialbus qt5serialport

define DEVICETEST_INSTALL_TARGET_CMDS
        mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
        $(INSTALL) -D -m 0644 $(@D)/icon_deviceTest.png $(TARGET_DIR)/usr/share/icon/
        $(INSTALL) -D -m 0755 $(@D)/DeviceTest $(TARGET_DIR)/usr/bin/
        $(INSTALL) -D -m 0755 $(@D)/DeviceTest.desktop $(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0644 $(@D)/deviceTest.ini $(TARGET_DIR)/etc/
endef

$(eval $(qmake-package))
