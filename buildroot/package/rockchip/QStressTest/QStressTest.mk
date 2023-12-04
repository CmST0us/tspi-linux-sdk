################################################################################
#
# QSTRESSTEST
#
################################################################################

QSTRESSTEST_VERSION = 1.0
QSTRESSTEST_SITE = $(TOPDIR)/../app/QStressTest
QSTRESSTEST_SITE_METHOD = local

QSTRESSTEST_LICENSE = Apache V2.0
QSTRESSTEST_LICENSE_FILES = NOTICE

define QSTRESSTEST_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define QSTRESSTEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QSTRESSTEST_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/image/QStressTest.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/QStressTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/QStressTest.desktop $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(generic-package))
