################################################################################
#
# dialserver
#
################################################################################

# add dependencies
#DIALSERVER_DEPENDENCIES = libcutils
ifeq ($(BR2_PACKAGE_RK3128H),y)
DIALSERVER_VERSION =
DIALSERVER_SITE = $(TOPDIR)/../external/dialserver
#$(info $(DIALSERVER_SITE))
DIALSERVER_SITE_METHOD = local
DIALSERVER_LICENSE = BSD-3c
DIALSERVER_LICENSE_FILES = LICENSE
DIALSERVER_INSTALL_STAGING = YES
DIALSERVER_CFLAGS ="$(TARGET_CFLAGS) -std=gnu99"

DIALSERVER_CONF_ENV = CFLAGS="$(TARGET_CFLAGS) -std=gnu99"
$(eval $(cmake-package))
endif
