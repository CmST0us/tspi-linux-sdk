################################################################################
#
# pkg-config
#
################################################################################

PKG_CONFIG_VERSION = 0.29.2
PKG_CONFIG_SITE = https://pkg-config.freedesktop.org/releases
PKG_CONFIG_SOURCE = pkg-config-$(PKG_CONFIG_VERSION).tar.gz
PKG_CONFIG_LICENSE = GPL Version 2
PKG_CONFIG_LICENSE_FILE = COPYING

PKG_CONFIG_INSTALL_STAGING = YES

$(eval $(autotools-package))
