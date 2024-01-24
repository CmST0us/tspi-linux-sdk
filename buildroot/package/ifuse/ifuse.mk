################################################################################
#
# ifuse
#
################################################################################

IFUSE_VERSION = 1.1.4
IFUSE_SOURCE = ifuse-$(IFUSE_VERSION).tar.bz2
IFUSE_SITE = https://github.com/libimobiledevice/ifuse/releases/download/$(IFUSE_VERSION)
IFUSE_LICENSE = LGPL-2.1+
IFUSE_LICENSE_FILES = COPYING
IFUSE_CPE_ID_VENDOR = ifuse

IFUSE_DEPENDENCIES = libfuse libimobiledevice

$(eval $(autotools-package))
