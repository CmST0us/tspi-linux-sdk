RV1108_MINIGUI_SITE = $(TOPDIR)/../external/libminigui
RV1108_MINIGUI_SITE_METHOD = local
RV1108_MINIGUI_INSTALL_STAGING = YES

RV1108_MINIGUI_LICENSE_FILES = COPYING
RV1108_MINIGUI_LICENSE = GPLv3

RV1108_MINIGUI_DEPENDENCIES = host-pkgconf libion rkfb rockchip-rga

RV1108_MINIGUI_CONF_OPTS = \
	--host=arm-linux \
	--build=i386-linux \
	--with-osname=linux \
	--with-targetname=rkfb \
	--enable-autoial \
	--disable-vbfsupport \
	--disable-textmode \
	--enable-vbfsupport \
	--disable-pcxvfb \
	--disable-dlcustomial \
	--disable-dummyial \
	--disable-fontcourier \
	--disable-screensaver \
	--disable-cisco_touchpad_ial \
	--enable-RKKeybroad_ial \
	--disable-fontsserif \
	--disable-fontsystem \
	--disable-flatlf \
	--disable-skinlf \
	--disable-mousecalibrate \
	--disable-dblclk \
	--disable-consoleps2 \
	--disable-consolems \
	--disable-consolems3 \
	--disable-cursor \
	--disable-videofbcon \
	--disable-rbfterminal \
	--disable-rbffixedsys \
	--disable-vbfsupport \
	--enable-videorkfb \
	--disable-splash \
	--enable-videoshadow

ifeq ($(BR2_PACKAGE_JPEG),y)
RV1108_MINIGUI_CONF_OPTS += --enable-jpgsupport
RV1108_MINIGUI_DEPENDENCIES += jpeg
else
RV1108_MINIGUI_CONF_OPTS += --disable-jpgsupport
endif

ifeq ($(BR2_PACKAGE_RV1108_LIBPNG),y)
RV1108_MINIGUI_CONF_OPTS += --enable-pngsupport
RV1108_MINIGUI_DEPENDENCIES += rv1108_libpng
else
RV1108_MINIGUI_CONF_OPTS += --disable-pngsupport
endif

ifeq ($(BR2_PACKAGE_TSLIB),y)
RV1108_MINIGUI_CONF_OPTS += --enable-tslibial
RV1108_MINIGUI_DEPENDENCIES += tslib
else
RV1108_MINIGUI_CONF_OPTS += --disable-tslibial
endif

ifeq ($(BR2_PACKAGE_FREETYPE),y)
RV1108_MINIGUI_CONF_OPTS += \
    --enable-ttfsupport \
    --with-ttfsupport=ft2 \
    --with-ft2-includes=$(STAGING_DIR)/usr/include/freetype2
RV1108_MINIGUI_DEPENDENCIES += freetype
endif

ifeq ($(BR2_PACKAGE_RV1108_MINIGUI_STATIC),y)
RV1108_MINIGUI_CONF_OPTS += \
    --disable-shared
else
RV1108_MINIGUI_CONF_OPTS += \
    --disable-static
endif

INCLUDE_PATH=$(STAGING_DIR)/usr/include/
define RV1108_MINIGUI_CONFIGURE_CMDS
    cd $(@D); $(TARGET_MAKE_ENV) ./configure $(RV1108_MINIGUI_CONF_OPTS) CFLAGS="$(CFLAGS) -I $(INCLUDE_PATH)"
endef

define RV1108_MINIGUI_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define RV1108_MINIGUI_INSTALL_STAGING_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(STAGING_DIR) -C $(@D) install
endef

define RV1108_MINIGUI_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(TARGET_DIR) -C $(@D) install && \
    rm -fr $(TARGET_DIR)/usr/local/include/ && \
    rm -fr $(TARGET_DIR)/usr/local/share/minigui/cmake && \
    rm $(TARGET_DIR)/usr/local/lib/*.a
    rm -fr $(TARGET_DIR)/usr/local/lib/pkgconfig
endef


$(eval $(generic-package))
