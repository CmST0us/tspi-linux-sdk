################################################################################
#
# libusbmuxd
#
################################################################################

LIBUSBMUXD_VERSION = 2.0.2
LIBUSBMUXD_SOURCE = libusbmuxd-$(LIBUSBMUXD_VERSION).tar.bz2
LIBUSBMUXD_SITE = https://github.com/libimobiledevice/libusbmuxd/releases/download/$(LIBUSBMUXD_VERSION)
LIBUSBMUXD_INSTALL_STAGING = YES
LIBUSBMUXD_LICENSE = LGPL-2.1+
LIBUSBMUXD_LICENSE_FILES = COPYING
LIBUSBMUXD_CPE_ID_VENDOR = libusbmuxd

LIBUSBMUXD_DEPENDENCIES = libplist

$(eval $(autotools-package))
