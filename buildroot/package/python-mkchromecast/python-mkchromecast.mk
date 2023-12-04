################################################################################
#
# python-mkchromecast
#
################################################################################

PYTHON_MKCHROMECAST_VERSION = 417876bd77926c941e46bf45be9bbeef6350e69c
PYTHON_MKCHROMECAST_SITE = $(call github,muammar,mkchromecast,$(PYTHON_MKCHROMECAST_VERSION))
PYTHON_MKCHROMECAST_SETUP_TYPE = setuptools
PYTHON_MKCHROMECAST_LICENSE = MIT
PYTHON_MKCHROMECAST_LICENSE_FILES = LICENSE

$(eval $(python-package))
