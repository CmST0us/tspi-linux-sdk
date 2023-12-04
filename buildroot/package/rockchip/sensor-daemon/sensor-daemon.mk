################################################################################
#
# sensor-daemon
#
################################################################################

SENSOR_DAEMON_VERSION = 1.0.0
SENSOR_DAEMON_SITE = $(TOPDIR)/../external/sensor-daemon
SENSOR_DAEMON_SITE_METHOD = local

define SENSOR_DAEMON_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 $(@D)/S99sensor-daemon $(TARGET_DIR)/etc/init.d/S99sensor-daemon
	$(INSTALL) -D -m 0755 $(@D)/sensor-daemon.conf $(TARGET_DIR)/etc/sensor-daemon.conf
endef

$(eval $(cmake-package))
