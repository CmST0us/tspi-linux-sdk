################################################################################
#
# extra-cmake-modules
#
################################################################################

EXTRA_CMAKE_MODULES_VERSION = 5.49.0
EXTRA_CMAKE_MODULES_SITE = https://github.com/KDE/extra-cmake-modules/archive
EXTRA_CMAKE_MODULES_SOURCE = v$(EXTRA_CMAKE_MODULES_VERSION).tar.gz
EXTRA_CMAKE_MODULES_LICENSE = BSD-3-Clause
EXTRA_CMAKE_MODULES_LICENSE_FILE = COPYING-CMAKE-SCRIPTS

EXTRA_CMAKE_MODULES_DEPENDENCIES = host-pkgconf
EXTRA_CMAKE_MODULES_INSTALL_STAGING = YES
EXTRA_CMAKE_MODULES_INSTALL_TARGET = NO

$(eval $(cmake-package))
