################################################################################
#
# alexa-smart-screen-sdk
#
################################################################################

ALEXA_SMART_SCREEN_SDK_VERSION = 0.5.0L
ALEXA_SMART_SCREEN_SDK_SITE = $(TOPDIR)/../external/avs-device-sdk
ALEXA_SMART_SCREEN_SDK_SITE_METHOD = local
ALEXA_SMART_SCREEN_SDK_LICENSE = BSD-3-Clause
ALEXA_SMART_SCREEN_SDK_LICENSE_FILE = BSD-3-Clause

ALEXA_SMART_SCREEN_SDK_DEPENDENCIES = nodejs avs-device-sdk apl-core-library
ALEXA_SMART_SCREEN_SDK_INSTALL_STAGING = YES

define ALEXA_SMART_SCREEN_SDK_CONFIGURE_CMDS
        (mkdir -p $(BUILD_DIR)/alexa-smart-screen-sdk-build && \
        cd $(BUILD_DIR)/alexa-smart-screen-sdk-build && \
        rm -f CMakeCache.txt && \
        PATH=$(BR_PATH) \
        $($(PKG)_CONF_ENV) $(BR2_CMAKE) $($(PKG)_SRCDIR) \
                -DCMAKE_TOOLCHAIN_FILE="$(HOST_DIR)/share/buildroot/toolchainfile.cmake" \
                -DCMAKE_INSTALL_PREFIX="/usr" \
                -DCMAKE_COLOR_MAKEFILE=OFF \
                -DCMAKE_COLOR_MAKEFILE=OFF \
                -DBUILD_TESTING=OFF \
                -DBUILD_SHARED_LIBS=$(if $(BR2_STATIC_LIBS),OFF,ON) \
		-DCMAKE_BUILD_TYPE=DEBUG \
                $(CMAKE_QUIET) \
                $($(PKG)_CONF_OPTS) \
        )
endef

define ALEXA_SMART_SCREEN_SDK_BUILD_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) -C $(BUILD_DIR)/alexa-smart-screen-sdk-build
endef

define ALEXA_SMART_SCREEN_SDK_STAGING_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_STAGING_OPTS) -C $(BUILD_DIR)/alexa-smart-screen-sdk-build
endef

define ALEXA_SMART_SCREEN_SDK_INSTALL_STAGING_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_TARGET_OPTS) -C $(BUILD_DIR)/alexa-smart-screen-sdk-build
endef

define ALEXA_SMART_SCREEN_SDK_INSTALL_TARGET_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_TARGET_OPTS) -C $(BUILD_DIR)/alexa-smart-screen-sdk-build
endef

$(eval $(cmake-package))
