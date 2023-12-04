################################################################################
#
# LUTRO
#
################################################################################
LIBRETRO_LUTRO_VERSION = b69dd3e566dfcffee17dd33049674b3d3dcd441c
LIBRETRO_LUTRO_SITE = $(call github,libretro,libretro-lutro,$(LIBRETRO_LUTRO_VERSION))

ifeq ($(BR2_arm),y)
	LIBRETRO_LUTRO_CONF_OPTS += HOST_CC="gcc -m32"
endif
ifeq ($(BR2_aarch64),y)
	LIBRETRO_LUTRO_CONF_OPTS += HOST_CC="gcc"
endif

define LIBRETRO_LUTRO_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       TARGET_STRIP=$(TARGET_STRIP) \
	       platform="unix" $(LIBRETRO_LUTRO_CONF_OPTS)
endef

define LIBRETRO_LUTRO_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/lutro_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/lutro_libretro.so
endef

$(eval $(generic-package))
