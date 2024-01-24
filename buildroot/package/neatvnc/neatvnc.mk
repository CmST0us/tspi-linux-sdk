################################################################################
#
# neatvnc
#
################################################################################

NEATVNC_SITE = https://github.com/any1/neatvnc.git
NEATVNC_VERSION = afe37b983db9b8fbf1189e07f756276e85a780e8
NEATVNC_SITE_METHOD = git
NEATVNC_LICENSE = ISC
NEATVNC_LICENSE_FILES = COPYING
NEATVNC_INSTALL_STAGING = YES

NEATVNC_DEPENDENCIES = aml pixman zlib

ifeq ($(BR2_PACKAGE_GNUTLS),y)
NEATVNC_DEPENDENCIES += gnutls
NEATVNC_CONF_OPTS += -Dtls=enabled
endif

ifeq ($(BR2_PACKAGE_JPEG_TURBO),y)
NEATVNC_DEPENDENCIES += jpeg-turbo
NEATVNC_CONF_OPTS += -Djpeg=enabled
endif

ifeq ($(BR2_PACKAGE_FFMPEG)$(BR2_PACKAGE_LIBDRM)$(BR2_PACKAGE_HAS_LIBGBM),yyy)
NEATVNC_DEPENDENCIES += ffmpeg libdrm libgbm
NEATVNC_CONF_OPTS += -Dh264=enabled
endif

$(eval $(meson-package))
