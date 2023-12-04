################################################################################
#
# MAME2010
#
################################################################################
LIBRETRO_MAME2010_VERSION = 70732f9137f6bb2bde4014746ea8bc613173dd1e
LIBRETRO_MAME2010_SITE = $(call github,libretro,mame2010-libretro,$(LIBRETRO_MAME2010_VERSION))

ifeq ($(BR2_arm),y)
	LIBRETRO_MAME2010_CONF += PTR64=0
endif

ifeq ($(BR2_aarch64),y)
	LIBRETRO_MAME2010_CONF += PTR64=1
endif

define LIBRETRO_MAME2010_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC) -lm" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)" $(LIBRETRO_MAME2010_CONF) 
endef

define LIBRETRO_MAME2010_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mame2010_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/mame2010_libretro.so
endef

$(eval $(generic-package))
