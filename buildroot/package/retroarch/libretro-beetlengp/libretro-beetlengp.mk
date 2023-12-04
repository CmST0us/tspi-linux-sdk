################################################################################
#
# BEETLENGP
#
################################################################################
LIBRETRO_BEETLENGP_VERSION = 69293c940ca27008ab2a1e37cc3077c677b36d1e
LIBRETRO_BEETLENGP_SITE = $(call github,libretro,beetle-ngp-libretro,$(LIBRETRO_BEETLENGP_VERSION))

define LIBRETRO_BEETLENGP_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_BEETLENGP_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_ngp_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlengp_libretro.so
endef

$(eval $(generic-package))
