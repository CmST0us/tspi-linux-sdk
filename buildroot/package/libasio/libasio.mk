################################################################################
#
# libasio
#
################################################################################

LIBASIO_VERSION = asio-1-14-0
LIBASIO_SITE = $(call github,chriskohlhoff,asio,$(LIBASIO_VERSION))
LIBASIO_SUBDIR = asio
LIBASIO_AUTORECONF = YES
# Headers only
LIBASIO_INSTALL_TARGET = NO
LIBASIO_INSTALL_STAGING = YES
LIBASIO_LICENSE = BSL-1.0
LIBASIO_LICENSE_FILES = asio/LICENSE_1_0.txt
LIBASIO_CONF_OPTS = --without-boost

ifeq ($(BR2_PACKAGE_OPENSSL),y)
LIBASIO_DEPENDENCIES += openssl
LIBASIO_CONF_OPTS += --with-openssl
else
LIBASIO_CONF_OPTS += --without-openssl
endif

$(eval $(autotools-package))
