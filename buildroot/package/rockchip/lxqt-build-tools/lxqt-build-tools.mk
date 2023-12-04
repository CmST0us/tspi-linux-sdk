################################################################################
#
# lxqt-build-tools
#
################################################################################

LXQT_BUILD_TOOLS_VERSION = 0.5.0
LXQT_BUILD_TOOLS_SITE = https://github.com/lxqt/lxqt-build-tools/releases/download/$(LXQT_BUILD_TOOLS_VERSION)
LXQT_BUILD_TOOLS_SOURCE = lxqt-build-tools-$(LXQT_BUILD_TOOLS_VERSION).tar.xz
LXQT_BUILD_TOOLS_LICENSE = BSD-3-Clause
LXQT_BUILD_TOOLS_LICENSE_FILE = BSD-3-Clause

LXQT_BUILD_TOOLS_DEPENDENCIES = pkg-config
LXQT_BUILD_TOOLS_INSTALL_STAGING = YES

define LXQT_BUILD_TOOLS_CONFIGURE_CMDS
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
                $(CMAKE_QUIET) \
                $($(PKG)_CONF_OPTS) \
        )
endef

define LXQT_BUILD_TOOLS_BUILD_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_BUILDDIR)build
endef

define LXQT_BUILD_TOOLS_INSTALL_STAGING_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_STAGING_OPTS) -C $($(PKG)_BUILDDIR)build
endef

define LXQT_BUILD_TOOLS_INSTALL_TARGET_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_TARGET_OPTS) -C $($(PKG)_BUILDDIR)build
endef

$(eval $(cmake-package))
