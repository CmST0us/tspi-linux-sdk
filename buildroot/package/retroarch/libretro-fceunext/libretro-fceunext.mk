################################################################################
#
# FCEUNEXT
#
################################################################################
LIBRETRO_FCEUNEXT_VERSION = ebd46a592c23a8091abd72e74cf0d9f4517769e1
LIBRETRO_FCEUNEXT_SITE = $(call github,libretro,fceu-next,$(LIBRETRO_FCEUNEXT_VERSION))

define LIBRETRO_FCEUNEXT_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D)/fceumm-code -f Makefile.libretro \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="unix"
endef

define LIBRETRO_FCEUNEXT_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/fceumm-code/fceumm_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/fceunext_libretro.so
endef

$(eval $(generic-package))
