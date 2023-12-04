################################################################################
#
# apl-core-library
#
################################################################################

APL_CORE_LIBRARY_VERSION = 0.5.0
APL_CORE_LIBRARY_SITE = $(TOPDIR)/../external/apl-core-library
APL_CORE_LIBRARY_SITE_METHOD = local
APL_CORE_LIBRARY_LICENSE = BSD-3-Clause
APL_CORE_LIBRARY_LICENSE_FILE = BSD-3-Clause

#APL_CORE_LIBRARY_DEPENDENCIES = 
APL_CORE_LIBRARY_INSTALL_STAGING = YES

define APL_CORE_LIBRARY_CONFIGURE_CMDS
        (mkdir -p  $($(PKG)_BUILDDIR)build&& \
        cd  $($(PKG)_BUILDDIR)build&& \
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
                -DBUILD_TESTS=ON \
                -DCMAKE_COLOR_MAKEFILE=OFF \
                -DBUILD_TESTING=OFF \
                -DBUILD_SHARED_LIBS=$(if $(BR2_STATIC_LIBS),OFF,ON) \
		-DCMAKE_BUILD_TYPE=DEBUG \
		-DCOVERAGE=OFF \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
                $(CMAKE_QUIET) \
                $($(PKG)_CONF_OPTS) \
        )
endef

define APL_CORE_LIBRARY_BUILD_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_BUILDDIR)build
endef

define APL_CORE_LIBRARY_INSTALL_STAGING_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_STAGING_OPTS) -C $($(PKG)_BUILDDIR)build
endef

define APL_CORE_LIBRARY_INSTALL_TARGET_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_TARGET_OPTS) -C $($(PKG)_BUILDDIR)build
endef

$(eval $(cmake-package))
