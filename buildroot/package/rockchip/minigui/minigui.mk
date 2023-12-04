MINIGUI_SITE = $(TOPDIR)/../external/minigui
MINIGUI_VERSION = master
MINIGUI_SITE_METHOD = local
MINIGUI_INSTALL_STAGING = YES

MINIGUI_LICENSE_FILES = COPYING
MINIGUI_LICENSE = GPLv3
MINIGUI_AUTORECONF = YES
MINIGUI_DEPENDENCIES = jpeg

ifeq ($(BR2_PACKAGE_MINIGUI_ENABLE_RGA),y)
MINIGUI_DEPENDENCIES += rockchip-rga
MINIGUI_CONF_ENV = CFLAGS+=" -DENABLE_RGA=1"
endif

##    --host=arm-linux
##    --target=arm-linux
MINIGUI_CONF_OPTS = \
    --build=i386-linux \
    --with-osname=linux \
    --disable-videopcxvfb \
    --with-ttfsupport=none \
    --disable-autoial \
    --disable-vbfsupport \
    --disable-textmode \
    --enable-vbfsupport \
    --disable-pcxvfb \
    --disable-dlcustomial \
    --disable-dummyial \
    --disable-jpgsupport \
    --disable-fontcourier \
    --disable-screensaver \
    --enable-RKKeyboard_ial \
    --enable-jpgsupport \
    --disable-fontsserif \
    --disable-fontsystem \
    --disable-flatlf \
    --disable-skinlfi \
    --disable-dblclk \
    --disable-consoleps2 \
    --disable-consolems \
    --disable-consolems3 \
    --disable-rbfterminal \
    --disable-rbffixedsys \
    --disable-vbfsupport \
    --disable-splash \
    --enable-videoshadow \
    --disable-static \
    --enable-shared \
    --disable-procs \
    --with-runmode=ths \
    --disable-incoreres \
    --disable-cursor \
    --enable-mousecalibrate \
    --with-pic

ifeq ($(BR2_PACKAGE_MINIGUI_SOFTWARE_SCALE),y)
MINIGUI_CONF_ENV += CFLAGS+=" -DDRM_VOP_SCALE=0"
MINIGUI_CONF_OPTS += --enable-pixman
MINIGUI_DEPENDENCIES += pixman
else
MINIGUI_CONF_ENV += CFLAGS+=" -DDRM_VOP_SCALE=1"
endif

ifeq ($(BR2_PACKAGE_LIBDRM),y)
MINIGUI_TARGET=drmcon
MINIGUI_CONF_OPTS += \
    --enable-videodrmcon \
    --disable-videofbcon

else
MINIGUI_TARGET=fbcon
MINIGUI_CONF_OPTS += \
    --enable-videofbcon
endif

MINIGUI_CONF_OPTS += \
    --with-targetname=$(MINIGUI_TARGET)

ifeq ($(BR2_PACKAGE_MINIGUI_ENABLE_FREETYPE),y)
MINIGUI_DEPENDENCIES += freetype
MINIGUI_CONF_OPTS += \
    --enable-ttfsupport \
    --with-ttfsupport=ft2 \
    --with-ft2-includes=$(STAGING_DIR)/usr/include/freetype2
endif

ifeq ($(BR2_PACKAGE_MINIGUI_ENABLE_PNG),y)
MINIGUI_DEPENDENCIES += libpng
MINIGUI_CONF_OPTS += \
    --enable-pngsupport
endif

ifeq ($(BR2_PACKAGE_TSLIB),y)
MINIGUI_DEPENDENCIES += tslib
MINIGUI_CONF_OPTS += \
    --enable-tslibial
endif


$(eval $(autotools-package))
