################################################################################
#
# qtxdg
#
################################################################################

QTXDG_VERSION = 3.2.0
QTXDG_SITE = https://github.com/lxqt/libqtxdg/releases/download/$(QTXDG_VERSION)
QTXDG_SOURCE = libqtxdg-$(QTXDG_VERSION).tar.xz
QTXDG_LICENSE = Digia Qt LGPL Exception version 1.1
QTXDG_LICENSE_FILE = Digia-Qt-LGPL-Exception-1.1
QTXDG_DEPENDENCIES = qt5base qt5svg

define QTXDG_CONFIGURE_CMDS
        (mkdir -p $($(PKG)_BUILDDIR)build && \
        cd $($(PKG)_BUILDDIR)build && \
        rm -f CMakeCache.txt && \
        PATH=$(BR_PATH) \
        $($(PKG)_CONF_ENV) $(BR2_CMAKE) $($(PKG)_SRCDIR) \
                -DCMAKE_TOOLCHAIN_FILE="$(HOST_DIR)/share/buildroot/toolchainfile.cmake" \
                -DCMAKE_INSTALL_PREFIX="/usr" \
                -DCMAKE_COLOR_MAKEFILE=OFF \
                -DBUILD_DOC=OFF \
                -DBUILD_DOCS=OFF \
                -DBUILD_EXAMPLE=OFF \
                -DBUILD_EXAMPLES=OFF \
                -DBUILD_TEST=OFF \
                -DBUILD_TESTS=OFF \
                -DBUILD_TESTING=OFF \
                -DBUILD_SHARED_LIBS=$(if $(BR2_STATIC_LIBS),OFF,ON) \
		-DQTXDGX_ICONENGINEPLUGIN_INSTALL_PATH="lib/qt/plugins/iconengines" \
                $(CMAKE_QUIET) \
                $($(PKG)_CONF_OPTS) \
        )
endef

define QTXDG_BUILD_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_BUILDDIR)build
endef

define QTXDG_INSTALL_TARGET_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_TARGET_OPTS) -C $($(PKG)_BUILDDIR)build
endef
$(eval $(cmake-package))
