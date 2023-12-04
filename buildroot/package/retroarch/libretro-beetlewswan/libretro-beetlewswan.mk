################################################################################
#
# BEETLEWSWAN
#
################################################################################
LIBRETRO_BEETLEWSWAN_VERSION = b4dc85b0ada2b27af3d4420439b0e4528363ef67
LIBRETRO_BEETLEWSWAN_SITE = $(call github,libretro,beetle-wswan-libretro,$(LIBRETRO_BEETLEWSWAN_VERSION))

define LIBRETRO_BEETLEWSWAN_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_BEETLEWSWAN_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_wswan_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlewswan_libretro.so
endef

$(eval $(generic-package))
