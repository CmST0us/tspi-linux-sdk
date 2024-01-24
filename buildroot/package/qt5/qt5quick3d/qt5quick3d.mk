################################################################################
#
# qt5quick3d
#
################################################################################

QT5QUICK3D_VERSION = 1d1420cd7e49dd588b8986952c0bcd6b7e6b83cb
QT5QUICK3D_SITE = $(QT5_SITE)/qtquick3d/-/archive/$(QT5QUICK3D_VERSION)
QT5QUICK3D_SOURCE = qtquick3d-$(QT5QUICK3D_VERSION).tar.bz2
QT5QUICK3D_DEPENDENCIES = qt5declarative
QT5QUICK3D_INSTALL_STAGING = YES
QT5QUICK3D_SYNC_QT_HEADERS = YES

ifeq ($(BR2_PACKAGE_ASSIMP),y)
QT5QUICK3D_DEPENDENCIES += assimp
endif

QT5QUICK3D_LICENSE = GPL-3.0
QT5QUICK3D_LICENSE_FILES = LICENSE.GPLv3

$(eval $(qmake-package))
