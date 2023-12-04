PCBA_TEST_SITE = $(TOPDIR)/../app/pcba_test
PCBA_TEST_SITE_METHOD = local
PCBA_TEST_INSTALL_STAGING = YES

# add dependencies
PCBA_TEST_DEPENDENCIES =

ifeq ($(BR2_PACKAGE_PCBA_TEST_PCTOOL_APP),y)
    PCBA_TEST_CONF_OPTS += -DUSE_PCBA_PCTOOL_APP=1
define PCBA_TEST_INSTALL_INIT_SYSV
    $(INSTALL) -m 0755 -D package/rockchip/rv1108_package/app/pcba_test/S80_pcba_server \
                    $(TARGET_DIR)/etc/init.d/
endef
endif

ifeq ($(BR2_PACKAGE_PCBA_TEST_SELTTEST_APP),y)
    PCBA_TEST_CONF_OPTS += -DUSE_PCBA_SELFTEST_APP=1
    PCBA_TEST_DEPENDENCIES += rv1108_minigui libjpeg libpng tslib
endif

$(eval $(cmake-package))
