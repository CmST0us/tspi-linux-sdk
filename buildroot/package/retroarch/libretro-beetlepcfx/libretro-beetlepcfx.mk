################################################################################
#
# BEETLEPCFX
#
################################################################################
LIBRETRO_BEETLEPCFX_VERSION = 84b7ca6af37525af126069f812d53784bc1702fe
LIBRETRO_BEETLEPCFX_SITE = $(call github,libretro,beetle-pcfx-libretro,$(LIBRETRO_BEETLEPCFX_VERSION))

define LIBRETRO_BEETLEPCFX_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_BEETLEPCFX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_pcfx_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlepcfx_libretro.so
endef

$(eval $(generic-package))
