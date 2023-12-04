################################################################################
#
# DOSBOX
#
################################################################################
LIBRETRO_DOSBOX_VERSION = 2d6bfbfe9ae82be17afbadf41f24950c2f9e2db9
LIBRETRO_DOSBOX_SITE = $(call github,libretro,dosbox-libretro,$(LIBRETRO_DOSBOX_VERSION))

ifeq ($(BR2_arm),y)
        LIBRETRO_DOSBOX_CONF += WITH_DYNAREC=arm
endif

ifeq ($(BR2_aarch64),y)
        LIBRETRO_DOSBOX_CONF += WITH_DYNAREC=arm64
endif

define LIBRETRO_DOSBOX_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D)/ -f Makefile.libretro \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="unix" $(LIBRETRO_DOSBOX_CONF)
endef

define LIBRETRO_DOSBOX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/dosbox_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/dosbox_libretro.so
endef

$(eval $(generic-package))
