################################################################################
#
# BEETLEPCEFAST
#
################################################################################
LIBRETRO_BEETLEPCEFAST_VERSION = f61ab365200b3e3eed56bf3fc27fdc879c5d5269
LIBRETRO_BEETLEPCEFAST_SITE = $(call github,libretro,beetle-pce-fast-libretro,$(LIBRETRO_BEETLEPCEFAST_VERSION))

define LIBRETRO_BEETLEPCEFAST_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_BEETLEPCEFAST_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_pce_fast_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlepcefast_libretro.so
endef

$(eval $(generic-package))
