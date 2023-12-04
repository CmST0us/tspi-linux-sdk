################################################################################
#
## smart_display_service from zw
#
#################################################################################


SMART_DISPLAY_SERVICE_SITE = $(TOPDIR)/../app/smart_display_service
SMART_DISPLAY_SERVICE_SITE_METHOD = local
SMART_DISPLAY_SERVICE_INSTALL_STAGING = NO

SMART_DISPLAY_SERVICE_CONF_OPTS += "-DCMAKE_BUILD_TYPE=Debug"

$(eval $(cmake-package))
