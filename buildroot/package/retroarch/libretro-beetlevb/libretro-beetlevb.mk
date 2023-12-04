################################################################################
#
# BEETLEVB
#
################################################################################
LIBRETRO_BEETLEVB_VERSION = cc11960675aaef4bb9c8e50b8ada6c81d9044d96
LIBRETRO_BEETLEVB_SITE = $(call github,libretro,beetle-vb-libretro,$(LIBRETRO_BEETLEVB_VERSION))

define LIBRETRO_BEETLEVB_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_BEETLEVB_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_vb_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlevb_libretro.so
endef

$(eval $(generic-package))
