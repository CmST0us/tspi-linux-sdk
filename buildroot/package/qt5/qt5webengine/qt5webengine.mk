################################################################################
#
# qt5webengine
#
################################################################################

QT5WEBENGINE_VERSION = $(QT5_VERSION)
QT5WEBENGINE_SITE = $(QT5_SITE)
QT5WEBENGINE_SOURCE = qtwebengine-$(QT5_SOURCE_TARBALL_PREFIX)-$(QT5WEBENGINE_VERSION).tar.xz
QT5WEBENGINE_DEPENDENCIES = ffmpeg libglib2 libvpx libxkbcommon opus webp \
	qt5declarative qt5webchannel host-bison host-flex host-gperf \
	host-pkgconf host-python
QT5WEBENGINE_INSTALL_STAGING = YES

include package/qt5/qt5webengine/chromium-latest.inc

QT5WEBENGINE_LICENSE = GPL-2.0 or LGPL-3.0 or GPL-3.0 or GPL-3.0 with exception
QT5WEBENGINE_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT \
	LICENSE.GPLv3 LICENSE.LGPL3 $(QT5WEBENGINE_CHROMIUM_LICENSE_FILES)

ifeq ($(BR2_PACKAGE_QT5BASE_EXAMPLES),y)
QT5WEBENGINE_DEPENDENCIES += qt5svg
endif

ifeq ($(BR2_PACKAGE_QT5BASE_XCB),y)
QT5WEBENGINE_DEPENDENCIES += xlib_libXScrnSaver xlib_libXcomposite \
	xlib_libXcursor xlib_libXi xlib_libXrandr xlib_libXtst
endif

QT5WEBENGINE_DEPENDENCIES += host-libpng host-libnss libnss

# Below host deps are needed by generate_colors_info tool
QT5WEBENGINE_DEPENDENCIES += host-libpng host-webp

ifeq ($(BR2_PACKAGE_QT5BASE_FONTCONFIG),y)
QT5WEBENGINE_DEPENDENCIES += host-freetype
endif

ifeq ($(BR2_PACKAGE_QT5BASE_JPEG),y)
QT5WEBENGINE_DEPENDENCIES += host-libjpeg
endif

QT5WEBENGINE_CONFIG += -ffmpeg

ifeq ($(BR2_PACKAGE_QT5WEBENGINE_WEBRTC),y)
QT5WEBENGINE_CONFIG += -webrtc
endif

ifeq ($(BR2_PACKAGE_QT5WEBENGINE_PROPRIETARY_CODECS),y)
QT5WEBENGINE_CONFIG += -proprietary-codecs
endif

ifeq ($(BR2_PACKAGE_QT5WEBENGINE_ALSA),y)
QT5WEBENGINE_DEPENDENCIES += alsa-lib
else
QT5WEBENGINE_CONF_OPTS += QT_CONFIG-=alsa
endif

QT5WEBENGINE_CONF_OPTS += -- $(QT5WEBENGINE_CONFIG)

# QtWebengine's build system uses python, but only supports python2. We work
# around this by forcing python2 early in the PATH, via a python->python2
# symlink.
QT5WEBENGINE_ENV = PATH=$(@D)/host-bin:$(BR_PATH)
define QT5WEBENGINE_PYTHON2_SYMLINK
	mkdir -p $(@D)/host-bin
	ln -sf $(HOST_DIR)/bin/python2 $(@D)/host-bin/python
endef
QT5WEBENGINE_PRE_CONFIGURE_HOOKS += QT5WEBENGINE_PYTHON2_SYMLINK

QT5WEBENGINE_ENV += NINJAFLAGS="-j$(PARALLEL_JOBS)"

define QT5WEBENGINE_CREATE_HOST_PKG_CONFIG
	sed s%@HOST_DIR@%$(HOST_DIR)%g $(QT5WEBENGINE_PKGDIR)/host-pkg-config.in > $(@D)/host-bin/host-pkg-config
	chmod +x $(@D)/host-bin/host-pkg-config
endef
QT5WEBENGINE_PRE_CONFIGURE_HOOKS += QT5WEBENGINE_CREATE_HOST_PKG_CONFIG
QT5WEBENGINE_ENV += GN_PKG_CONFIG_HOST=$(@D)/host-bin/host-pkg-config

QT5WEBENGINE_CONF_ENV = $(QT5WEBENGINE_ENV)
QT5WEBENGINE_MAKE_ENV = $(QT5WEBENGINE_ENV)

define QT5WEBENGINE_INSTALL_TARGET_ENV
	$(INSTALL) -D -m 644 $(QT5WEBENGINE_PKGDIR)/qtwebengine.sh \
		$(TARGET_DIR)/etc/profile.d/qtwebengine.sh
endef
QT5WEBENGINE_POST_INSTALL_TARGET_HOOKS += QT5WEBENGINE_INSTALL_TARGET_ENV

ifeq ($(BR2_PACKAGE_LIBV4L_RKMPP),y)
define QT5WEBENGINE_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 $(QT5WEBENGINE_PKGDIR)/S99qtwebengine.sh \
		$(TARGET_DIR)/etc/init.d/S99qtwebengine.sh
endef
endif

$(eval $(qmake-package))
