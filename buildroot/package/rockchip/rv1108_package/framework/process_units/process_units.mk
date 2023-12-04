PROCESS_UNITS_SITE = $(TOPDIR)/../framework/process_units
PROCESS_UNITS_SITE_METHOD = local
PROCESS_UNITS_INSTALL_STAGING = YES

# add dependencies
PROCESS_UNITS_DEPENDENCIES = rkcamera rkfb rockchip-rga

PROCESS_UNITS_CONF_OPTS += \
    -DBOARD_VERSION=rv1108-$(RK_TARGET_BOARD_VERSION)

$(eval $(cmake-package))
