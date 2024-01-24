################################################################################
#
# libimobiledevice-glue
#
################################################################################

LIBIMOBILEDEVICE_GLUE_VERSION = ecb0996fd2a3b0539153dd3ef901d137bf498ffe
LIBIMOBILEDEVICE_GLUE_SITE = $(call github,libimobiledevice,libimobiledevice-glue,$(LIBIMOBILEDEVICE_GLUE_VERSION))
LIBIMOBILEDEVICE_GLUE_INSTALL_STAGING = YES
LIBIMOBILEDEVICE_GLUE_LICENSE = LGPL-2.1+
LIBIMOBILEDEVICE_GLUE_LICENSE_FILES = COPYING
LIBIMOBILEDEVICE_GLUE_CPE_ID_VENDOR = libimobiledevice

LIBIMOBILEDEVICE_GLUE_AUTORECONF = YES

LIBIMOBILEDEVICE_GLUE_DEPENDENCIES = libplist

$(eval $(autotools-package))
