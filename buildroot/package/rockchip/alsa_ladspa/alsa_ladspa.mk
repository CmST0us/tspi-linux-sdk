################################################################################
#
# Rockchip ALSA LADSPA
#
################################################################################

ALSA_LADSPA_VERSION = 1.1
ALSA_LADSPA_SITE = $(TOPDIR)/../external/ladspaSDK
ALSA_LADSPA_SITE_METHOD = local

ALSA_LADSPA_LICENSE = Apache V2.0
ALSA_LADSPA_LICENSE_FILES = NOTICE

CXX="$(TARGET_CXX)"
PROJECT_DIR="$(@D)/src"
ALSA_LADSPA_BUILD_OPTS=-I$(PROJECT_DIR) -fPIC -I$(PROJECT_DIR)/include \
        --sysroot=$(STAGING_DIR) \
	-lpthread -ldl 

ALSA_LADSPA_MAKE_OPTS = \
        CPPFLAGS="$(TARGET_CPPFLAGS) $(ALSA_LADSPA_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)/src"


ifeq ($(call qstrip,$(BR2_ARCH)),aarch64)
define ALSA_LADSPA_BUILD_CMDS
        $(TARGET_MAKE_ENV) $(MAKE) LADSPA_PATH=libs_64 CC=$(TARGET_CC)  CXX="$(TARGET_CXX)" LD=$(TARGET_LD)  $(ALSA_LADSPA_MAKE_OPTS) -C $(@D)/src
endef

define ALSA_LADSPA_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 644 $(@D)/src/libs_64/Equalizer.so $(TARGET_DIR)/usr/lib/Equalizer.so
endef
endif

ifeq ($(call qstrip,$(BR2_ARCH)),arm)
define ALSA_LADSPA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) LADSPA_PATH=libs_32 CC=$(TARGET_CC) CXX="$(TARGET_CXX)" LD=$(TARGET_LD) $(ALSA_LADSPA_MAKE_OPTS) -C $(@D)/src
endef

define ALSA_LADSPA_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 644 $(@D)/src/libs_32/Equalizer.so $(TARGET_DIR)/usr/lib/Equalizer.so
endef
endif

$(eval $(generic-package))
