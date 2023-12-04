################################################################################
#
# avs-device-sdk
#
################################################################################

AVS_DEVICE_SDK_VERSION = 0.5.0
AVS_DEVICE_SDK_SITE = $(TOPDIR)/../external/avs-device-sdk
AVS_DEVICE_SDK_SITE_METHOD = local
AVS_DEVICE_SDK_LICENSE = BSD-3-Clause
AVS_DEVICE_SDK_LICENSE_FILE = BSD-3-Clause

AVS_DEVICE_SDK_DEPENDENCIES = libcurl nghttp2 libgcrypt libsoup portaudio host-doxygen
AVS_DEVICE_SDK_INSTALL_STAGING = YES

define AVS_DEVICE_SDK_CONFIGURE_CMDS
        (mkdir -p $(BUILD_DIR)/avs-device-sdk-build && \
        cd $(BUILD_DIR)/avs-device-sdk-build && \
        rm -f CMakeCache.txt && \
        PATH=$(BR_PATH) \
        $($(PKG)_CONF_ENV) $(BR2_CMAKE) $($(PKG)_SRCDIR) \
                -DCMAKE_TOOLCHAIN_FILE="$(HOST_DIR)/share/buildroot/toolchainfile.cmake" \
                -DCMAKE_INSTALL_PREFIX="/usr" \
                -DCMAKE_COLOR_MAKEFILE=OFF \
                -DCMAKE_COLOR_MAKEFILE=OFF \
                -DBUILD_TESTING=OFF \
                -DBUILD_SHARED_LIBS=$(if $(BR2_STATIC_LIBS),OFF,ON) \
		-DGSTREAMER_MEDIA_PLAYER=ON \
		-DPORTAUDIO=ON \
		-DPORTAUDIO_LIB_PATH=$(TARGET_DIR)/usr/lib/libportaudio.so \
		-DPORTAUDIO_INCLUDE_DIR=$(STAGING_DIR)/usr/include \
		-DCMAKE_BUILD_TYPE=DEBUG \
		-DRAPIDJSON_MEM_OPTIMIZATION=OFF \
                $(CMAKE_QUIET) \
                $($(PKG)_CONF_OPTS) \
        )
endef

define AVS_DEVICE_SDK_BUILD_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) -C $(BUILD_DIR)/avs-device-sdk-build
endef

define AVS_DEVICE_SDK_INSTALL_STAGING_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_STAGING_OPTS) -C $(BUILD_DIR)/avs-device-sdk-build
endef

define AVS_DEVICE_SDK_INSTALL_TARGET_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_TARGET_OPTS) -C $(BUILD_DIR)/avs-device-sdk-build
endef

$(eval $(cmake-package))
