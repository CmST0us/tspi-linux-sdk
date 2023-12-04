################################################################################
#
# VBANEXT
#
################################################################################
LIBRETRO_VBANEXT_VERSION = 3726a6f85c120aae42c206739d80bebab26405d0
LIBRETRO_VBANEXT_SITE = $(call github,libretro,vba-next,$(LIBRETRO_VBANEXT_VERSION))

define LIBRETRO_VBANEXT_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_VBANEXT_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/vba_next_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/vbanext_libretro.so
endef

$(eval $(generic-package))
