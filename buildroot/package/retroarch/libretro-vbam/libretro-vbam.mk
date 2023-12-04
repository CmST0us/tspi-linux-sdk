################################################################################
#
# VBAM
#
################################################################################
LIBRETRO_VBAM_VERSION = 8041717e1c9993085754b646d3c62ef9422ad652
LIBRETRO_VBAM_SITE = $(call github,libretro,vbam-libretro,$(LIBRETRO_VBAM_VERSION))

LIBRETRO_VBAM_CONF_OPTS += -DENABLE_SDL=OFF
LIBRETRO_VBAM_CONF_OPTS += -DENABLE_GTK=OFF
LIBRETRO_VBAM_CONF_OPTS += -DENABLE_OPENGL=OFF
LIBRETRO_VBAM_CONF_OPTS += -DCMAKE_CXX_FLAGS="-fpermissive"

define LIBRETRO_VBAM_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/libvbamcore.so \
		$(TARGET_DIR)/usr/lib/libretro/vbam_libretro.so
endef

$(eval $(cmake-package))
