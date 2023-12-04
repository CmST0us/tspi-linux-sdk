################################################################################
#
# PRBOOM
#
################################################################################
LIBRETRO_PRBOOM_VERSION = 3ff6c1c30ed4f23e2eba4f13a5a4851d401badbe
LIBRETRO_PRBOOM_SITE = $(call github,libretro,libretro-prboom,$(LIBRETRO_PRBOOM_VERSION))

define LIBRETRO_PRBOOM_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_PRBOOM_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/prboom_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/prboom_libretro.so
endef

$(eval $(generic-package))
