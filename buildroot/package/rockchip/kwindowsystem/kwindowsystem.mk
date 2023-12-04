################################################################################
#
# kwindowsystem
#
################################################################################

KWINDOWSYSTEM_VERSION = 5.49.0
KWINDOWSYSTEM_SITE = https://github.com/KDE/kwindowsystem/archive
KWINDOWSYSTEM_SOURCE = v$(KWINDOWSYSTEM_VERSION).tar.gz
KWINDOWSYSTEM_LICENSE = LGPL v2.1
KWINDOWSYSTEM_LICENSE_FILE = COPYING.LIB

KWINDOWSYSTEM_DEPENDENCIES = extra-cmake-modules
KWINDOWSYSTEM_INSTALL_STAGING = YES

$(eval $(cmake-package))
