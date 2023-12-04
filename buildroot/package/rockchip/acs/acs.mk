###############################################################################
#
# acs
#
################################################################################

ACS_VERSION = 1.16.0
ACS_SITE = $(TOPDIR)/../external/acs
ACS_SITE_METHOD = local
ACS_LICENSE = BSD-3-Clause
ACS_LICENSE_FILE = BSD-3-Clause

ACS_DEPENDENCIES = sysklogd sbc libcurl nghttp2 libgcrypt libsoup portaudio host-doxygen
AVS_DEVICE_SDK_INSTALL_STAGING = YES

define ACS_CONFIGURE_CMDS
        (mkdir -p $(@D)/ace/sdk/avs_device_sdk_build && cd $(@D)/ace/sdk/avs_device_sdk_build && rm -f CMakeCache.txt && \
        PATH=$(BR_PATH) \
        $($(PKG)_CONF_ENV) $(BR2_CMAKE) $($(PKG)_SRCDIR)/ace/sdk/sdk/avs_device_sdk/repo \
                -DCMAKE_TOOLCHAIN_FILE="$(HOST_DIR)/share/buildroot/toolchainfile.cmake" \
                -DCMAKE_INSTALL_PREFIX="/usr" \
                -DCMAKE_COLOR_MAKEFILE=OFF \
                -DCMAKE_VERBOSE_MAKEFILE=ON \
                -DBUILD_TESTING=OFF \
                -DBUILD_SHARED_LIBS=$(if $(BR2_STATIC_LIBS),OFF,ON) \
                -DGSTREAMER_MEDIA_PLAYER=ON \
                -DPORTAUDIO=ON \
                -DPORTAUDIO_LIB_PATH=$(TARGET_DIR)/usr/lib/libportaudio.so \
                -DPORTAUDIO_INCLUDE_DIR=$(STAGING_DIR)/usr/include \
                -DCMAKE_BUILD_TYPE=DEBUG \
                -DACS_SDK_PATH="$($(PKG)_SRCDIR)/ace/sdk" \
                -DACS_UTILS=ON \
                -DEXTENSION_PATH="$($(PKG)_SRCDIR)/ace/sdk/sdk/avs_device_sdk/ACSUtilities" \
                -DRAPIDJSON_MEM_OPTIMIZATION=OFF \
		-DACS_SDK_V123_AND_BELOW=ON \
		-DBLUETOOTH_ENABLED=ON \
		-DACS_SDK_BLUETOOTH=ON \
		-DBUILD_TESTING=ON \
                $(CMAKE_QUIET) \
                $($(PKG)_CONF_OPTS) \
	)
endef

define ACS_BUILD_CMDS
	ln -fs $(STAGING_DIR) $(@D)/sysroot
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/log CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include"
	$(TARGET_MAKE_ENV) $(MAKE) ../../../../ace/sdk/lib/libacehal_kv_storage.so -C $(@D)/dpk_impl/raspi/ace_hal/kv_storage CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/dpk_impl/raspi/ace_hal/kv_storage/include"
	$(TARGET_MAKE_ENV) $(MAKE) ../../../../ace/sdk/bin/kvs_util -C $(@D)/dpk_impl/raspi/ace_hal/kv_storage CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/dpk_impl/raspi/ace_hal/kv_storage/include"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/device_info CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/dpk_impl/raspi/ace_hal/device_info/include -I$(@D)/dpk_impl/raspi/ace_hal/log"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/wifi CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -g -O2 -fPIC -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/ace/sdk/include/ace -I$(@D)/dpk_impl/raspi/ace_hal/log -I$(@D)/dpk_impl/raspi/ace_hal/wifi/include -I$(@D)/dpk_impl/raspi/ace_hal/wifi/include/nm -I$(STAGING_DIR)/usr/include/libnm -I$(STAGING_DIR)/usr/include/glib-2.0 -I$(STAGING_DIR)/usr/lib/glib-2.0/include"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/factory_reset CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -fPIC -Wall --std=gnu11 -Wextra -Wno-unused -Wno-unused-parameter -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/dpk_impl/raspi/ace_hal/kv_storage/include -I$(@D)/dpk_impl/raspi/ace_hal/log -I$(@D)/dpk_impl/raspi/ace_hal/factory_reset/include"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/dha CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -fPIC -Wall --std=gnu11 -Wextra -Werror -Wno-unused -Wno-unused-parameter -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/dpk_impl/raspi/ace_hal/dha/common -I$(@D)/dpk_impl/raspi/ace_hal/log -I$(@D)/dpk_impl/raspi/ace_hal/dha/crypto_utils -I$(@D)/dpk_impl/raspi/ace_hal/dha/port -I$(@D)/dpk_impl/raspi/ace_hal/dha/rpi_hw"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon CC="$(TARGET_CC)" ACS_DPK_ROOT_DIR="$(@D)" CFLAGS="$(TARGET_CFLAGS) -fPIC -Wno-return-type -Wno-unused -Wno-unused-parameter -pthread -DHAVE_CONFIG_H -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon/include -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon/src  -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon/shared -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon/lib -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon/attrib -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_daemon/btio -I$(STAGING_DIR)/usr/include/glib-2.0 -I$(STAGING_DIR)/usr/lib/glib-2.0/include -I$(STAGING_DIR)/usr/include/dbus-1.0 -I$(STAGING_DIR)/usr/lib/dbus-1.0/include"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_hal CC="$(TARGET_CC)" ACS_DPK_ROOT_DIR="$(@D)" BLE_DAEMON_NAME="bluezbled" CFLAGS="$(TARGET_CFLAGS) -fPIC -Wall -g --std=gnu11 -Wno-return-type -Wno-error -Wno-unused -Wno-unused-parameter -pedantic -pthread -Wno-deprecated-declarations -Wno-pointer-arith -DBLE_DAEMON_NAME="\\\"bluezbled\\\"" -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/dpk_impl/raspi/ace_hal/bluez_ble/bluez_ble_hal/include"
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dpk_impl/raspi/ace_hal/bt CC="$(TARGET_CC)" ACS_DPK_ROOT_DIR="$(@D)" CFLAGS="$(TARGET_CFLAGS) -Wall -std=gnu99 -fPIC -pthread -I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include -I$(@D)/ace/sdk/include -I$(@D)/ace/sdk/include/ace -I$(@D)/dpk_impl/raspi/ace_hal/bt/include -I$(@D)/dpk_impl/raspi/ace_hal/bt/include/co -I$(@D)/dpk_impl/raspi/ace_hal/bt/include/bluez -I$(@D)/dpk_impl/raspi/ace_hal/bt/include/bluez/profile/a2dp -I$(@D)/dpk_impl/raspi/ace_hal/bt/include/bluealsa -I$(@D)/dpk_impl/raspi/ace_hal/bt/include/bluez/gatt -I$(@D)/dpk_impl/raspi/ace_hal/log -I$(STAGING_DIR)/usr/include/glib-2.0 -I$(STAGING_DIR)/usr/lib/glib-2.0/include -I$(STAGING_DIR)/usr/include/dbus-1.0 -I$(STAGING_DIR)/usr/lib/dbus-1.0/include -I$(STAGING_DIR)/usr/include/alsa"
        $(TARGET_MAKE_ENV) $(MAKE) -j1 -C $(@D)/ace/sdk PLATFORM_CROSS_PATH="$(HOST_DIR)/bin" PLATFORM_TOOLCHAIN_PREFIX="$(GNU_TARGET_NAME)" LIBRARY_PATH="$(@D)/ace/sdk" PLATFORM_CROSS_SYSROOT="$(STAGING_DIR)" PLATFORM_CFLAGS="$(TARGET_CFLAGS) -DAIPC_SKIP_ROOT_CHK" PLATFORM_CXXFALGS="$(TARGET_CFLAGS)" CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)" PLATFORM_INCLUDES="-I$(STAGING_DIR)/usr/include -I$(TARGET_DIR)/usr/include" WERROR_ENABLED="n"
	$(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) -C $(@D)/ace/sdk/avs_device_sdk_build
	#$(TARGET_CC) -o $(TARGET_DIR)/usr/bin/device_info $(@D)/ace/sdk/objs/libace_device_info/dpk/hal/cli/device_info/device_info_cli.o -fPIC -Wall --std=gnu11 -Wextra -Werror -Wno-unused -Wno-unused-parameter -I$(@D)/ace/sdk/include -L$(@D)/ace/sdk/lib/ -lacehal_device_info -lacehal_kv_storage -lace_log -lacehal_log
endef

define ACS_INSTALL_STAGING_CMDS
        $(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_STAGING_OPTS) -C $(@D)/ace/sdk/avs_device_sdk_build
endef

define ACS_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/lib/data/ace $(TARGET_DIR)/var/lib/data/ace/models $(TARGET_DIR)/var/lib/data/ace/kvstorage $(TARGET_DIR)/databases $(TARGET_DIR)/etc/alsa/conf.d
	chmod -R 777 $(TARGET_DIR)/var/lib/data $(TARGET_DIR)/databases
	$(SED) 's/messages/syslog/' $(TARGET_DIR)/etc/syslog.conf
	$(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) $($(PKG)_INSTALL_TARGET_OPTS) -C $(@D)/ace/sdk/avs_device_sdk_build
	$(INSTALL) -D -m 755 $(TOPDIR)/package/rockchip/acs/S01login $(TARGET_DIR)/etc/init.d/
	$(INSTALL) -D -m 755 $(TOPDIR)/package/rockchip/acs/S99acs $(TARGET_DIR)/etc/init.d/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/bin/* $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/lib/* $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/SampleApp/src/SampleApp $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/AlertsIntegrationTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/AlexaDirectiveSequencerLibraryTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/SpeechSynthesizerIntegrationTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/ServerDisconnectIntegrationTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/AudioPlayerIntegrationTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/AudioInputProcessorIntegrationTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/AlexaCommunicationsLibraryTest $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/ace/sdk/avs_device_sdk_build/Integration/test/AlexaAuthorizationDelegateTest $(TARGET_DIR)/usr/bin/
#	$(INSTALL) -D -m 755 $(@D)/ace/sdk/sdk/avs_device_sdk/testapp/TestApp $(TARGET_DIR)/usr/bin/
#	$(INSTALL) -D -m 755 $(@D)/ace/sdk/sdk/avs_device_sdk/testapp/DirectiveInjectorTest $(TARGET_DIR)/usr/bin/
#	$(INSTALL) -D -m 755 $(@D)/ace/sdk/sdk/avs_device_sdk/testapp/lib/* $(TARGET_DIR)/usr/lib/
#	$(INSTALL) -D -m 755 $(@D)/ace/sdk/sdk/avs_device_sdk/testapp/models/* $(TARGET_DIR)/var/lib/data/ace/models/
	$(INSTALL) -D -m 755 $(@D)/scripts/rpi/aipc-configuration.sh $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/AlexaClientSDKConfig.json $(TARGET_DIR)/var/lib/data/ace/
	$(INSTALL) -D -m 755 $(@D)/scripts/rpi/config_files/NetworkManager.conf $(TARGET_DIR)/etc/NetworkManager
	echo -e 'denyinterfaces wlan0' >> $(TARGET_DIR)/etc/dhcpcd.conf
	$(INSTALL) -D -m 755 $(@D)/scripts/rpi/config_files/20-bluealsa.conf $(TARGET_DIR)/etc/alsa/conf.d/
	$(INSTALL) -D -m 755 $(@D)/scripts/rpi/config_files/bluealsa.conf $(TARGET_DIR)/etc/dbus-1/system.d/
	$(INSTALL) -D -m 755 $(@D)/scripts/rpi/device-provisioning/cert.pem $(TARGET_DIR)/var/lib/data/ace/
	$(INSTALL) -D -m 755 $(@D)/scripts/rpi/device-provisioning/device_info.json $(TARGET_DIR)/var/lib/data/ace/
	rm -rf $(TARGET_DIR)/etc/resolv.conf	
endef

$(eval $(cmake-package))
