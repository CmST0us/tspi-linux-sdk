################################################################################
#
# libimobiledevice
#
################################################################################

LIBIMOBILEDEVICE_VERSION = 1.3.0
LIBIMOBILEDEVICE_SOURCE = libimobiledevice-$(LIBIMOBILEDEVICE_VERSION).tar.bz2
LIBIMOBILEDEVICE_SITE = https://github.com/libimobiledevice/libimobiledevice/releases/download/$(LIBIMOBILEDEVICE_VERSION)
LIBIMOBILEDEVICE_INSTALL_STAGING = YES
LIBIMOBILEDEVICE_LICENSE = LGPL-2.1+
LIBIMOBILEDEVICE_LICENSE_FILES = COPYING
LIBIMOBILEDEVICE_CPE_ID_VENDOR = libimobiledevice

LIBIMOBILEDEVICE_DEPENDENCIES = libusbmuxd libimobiledevice-glue

# Disable building Python bindings, because it requires host-cython, which
# is not packaged in Buildroot at all.
LIBIMOBILEDEVICE_CONF_OPTS = --without-cython

$(eval $(autotools-package))
