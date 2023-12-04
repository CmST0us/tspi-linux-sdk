################################################################################
#
# UAE
#
################################################################################
LIBRETRO_UAE_VERSION = d7aadfb11eef48d5428fe9349d313b48fe8b41ef
LIBRETRO_UAE_SITE = $(call github,libretro,libretro-uae,$(LIBRETRO_UAE_VERSION))

define LIBRETRO_UAE_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS) -DFTIME" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="rpi"
endef

define LIBRETRO_UAE_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/puae_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/uae_libretro.so
endef

$(eval $(generic-package))
