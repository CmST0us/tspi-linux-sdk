################################################################################
#
# Rockchip CAE APP USING VAD
#
################################################################################

CAE_VAD_VERSION = 1.0
CAE_VAD_SITE = $(TOPDIR)/../external/cae_vad
CAE_VAD_SITE_METHOD = local

CAE_VAD_LICENSE = Apache V2.0
CAE_VAD_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
PROJECT_DIR="$(@D)"
CAE_VAD_BUILD_OPTS=-I$(PROJECT_DIR) -fPIC -O3 -I$(PROJECT_DIR)/include \
	--sysroot=$(STAGING_DIR) \
	-ldl -lpthread -lasound

CAE_VAD_MAKE_OPTS = \
        CPPFLAGS="$(TARGET_CPPFLAGS) $(CAE_VAD_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define CAE_VAD_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" $(CAE_VAD_MAKE_OPTS)
endef

define CAE_VAD_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 755 $(@D)/cae_vad $(TARGET_DIR)/usr/bin/
        $(INSTALL) -D -m 644 $(@D)/libcae.so $(TARGET_DIR)/usr/lib/
        $(INSTALL) -D -m 644 $(@D)/libIvw60.so $(TARGET_DIR)/usr/lib/
        $(INSTALL) -D -m 644 $(@D)/wozai.wav $(TARGET_DIR)/usr/lib/
        $(INSTALL) -D -m 644 $(@D)/ivw_resource.jet $(TARGET_DIR)/etc/
endef

$(eval $(generic-package))
