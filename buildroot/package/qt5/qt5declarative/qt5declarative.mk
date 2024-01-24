################################################################################
#
# qt5declarative
#
################################################################################

QT5DECLARATIVE_VERSION = 51efb2ed2f071beda188270a23ac450fe4b318f7
QT5DECLARATIVE_SITE = $(QT5_SITE)/qtdeclarative/-/archive/$(QT5DECLARATIVE_VERSION)
QT5DECLARATIVE_SOURCE = qtdeclarative-$(QT5DECLARATIVE_VERSION).tar.bz2
QT5DECLARATIVE_INSTALL_STAGING = YES
QT5DECLARATIVE_SYNC_QT_HEADERS = YES

QT5DECLARATIVE_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
QT5DECLARATIVE_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE_SOFTWARE_BACKEND),y)
define QT5DECLARATIVE_INSTALL_TARGET_ENV
	echo "export QT_QUICK_BACKEND=software" > \
		$(TARGET_DIR)/etc/profile.d/qtdeclarative.sh
endef
QT5DECLARATIVE_POST_INSTALL_TARGET_HOOKS += QT5DECLARATIVE_INSTALL_TARGET_ENV
endif

$(eval $(qmake-package))
