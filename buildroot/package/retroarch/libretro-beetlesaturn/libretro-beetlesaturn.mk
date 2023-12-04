################################################################################
#
# BEETLESATURN
#
################################################################################
LIBRETRO_BEETLESATURN_VERSION = 1983713f665e86459900a08873fac09e70c31bfa
LIBRETRO_BEETLESATURN_SITE = $(call github,libretro,beetle-saturn-libretro,$(LIBRETRO_BEETLESATURN_VERSION))

ifeq ($(BR2_arm)$(BR2_aarch64),y)
	LIBRETRO_BEETLESATURN_CONF_OPTS += IS_X86=0
endif

define LIBRETRO_BEETLESATURN_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)" \
	       $(LIBRETRO_BEETLESATURN_CONF_OPTS)
endef

define LIBRETRO_BEETLESATURN_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_saturn_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlesaturn_libretro.so
endef

$(eval $(generic-package))
