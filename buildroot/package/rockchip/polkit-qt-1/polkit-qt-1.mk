################################################################################
#
# polkit-qt-1
#
################################################################################

POLKIT_QT_1_VERSION = 0.112.0
POLKIT_QT_1_SITE = https://github.com/KDE/polkit-qt-1/archive
POLKIT_QT_1_SOURCE = v$(POLKIT_QT_1_VERSION).tar.gz
POLKIT_QT_1_LICENSE = LGPL v2.1
POLKIT_QT_1_LICENSE_FILE = COPYING

POLKIT_QT_1_DEPENDENCIES = qt5base polkit libglib2
POLKIT_QT_1_INSTALL_STAGING = YES

$(eval $(cmake-package))
