################################################################################
#
# MGBA
#
################################################################################
LIBRETRO_MGBA_VERSION = b03412aa38b39190f475cb89d5c8a2a5e97bc32d
LIBRETRO_MGBA_SITE = $(call github,mgba-emu,mgba,$(LIBRETRO_MGBA_VERSION))

# Configs for libretro
LIBRETRO_MGBA_CONF_OPTS += -DBUILD_LIBRETRO=ON
LIBRETRO_MGBA_CONF_OPTS += -DBUILD_QT=OFF
LIBRETRO_MGBA_CONF_OPTS += -DBUILD_SDL=OFF

# Reusing raspi configs for ARM
ifeq ($(BR2_arm),y)
	LIBRETRO_MGBA_CONF_OPTS += -DBUILD_RASPI=ON
endif

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	LIBRETRO_MGBA_CONF_OPTS += -DBUILD_GLES2=ON
endif

define LIBRETRO_MGBA_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mgba_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/mgba_libretro.so
endef

$(eval $(cmake-package))
