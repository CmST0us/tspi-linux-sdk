################################################################################
#
# led_control_app
#
################################################################################

LED_CONTROL_APP_LICENSE_FILES = NOTICE
LED_CONTROL_APP_LICENSE = Apache V2.0
LED_CONTROL_APP_SITE = $(TOPDIR)/package/rockchip/led_control_app
LED_CONTROL_APP_SITE_METHOD = local
LED_CONTROL_APP_LICENSE = Apache V2.0
LED_CONTROL_APP_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
PROJECT_DIR="$(@D)"
LED_CONTROL_APP_BUILD_OPTS=-I$(PROJECT_DIR) -fPIC \
	--sysroot=$(STAGING_DIR) \
	-ldl -lpthread

LED_CONTROL_APP_MAKE_OPTS = \
        CXXFLAGS="$(TARGET_CPPFLAGS) $(LED_CONTROL_APP_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define LED_CONTROL_APP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" $(LED_CONTROL_APP_MAKE_OPTS)
endef

define LED_CONTROL_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/led_control_app $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))
