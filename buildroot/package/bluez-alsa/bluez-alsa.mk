################################################################################
#
# bluez-alsa
#
################################################################################

BLUEZ_ALSA_VERSION = 22ffb1965a0b79fbb28af5751b98814f94f6f81d
BLUEZ_ALSA_SITE = $(call github,Arkq,bluez-alsa,$(BLUEZ_ALSA_VERSION))
BLUEZ_ALSA_LICENSE = MIT
BLUEZ_ALSA_LICENSE_FILES = LICENSE.txt
BLUEZ_ALSA_DEPENDENCIES = alsa-lib bluez5_utils libglib2 sbc host-pkgconf

# git repo, no configure
BLUEZ_ALSA_AUTORECONF = YES

# Autoreconf requires an existing m4 directory
define BLUEZ_ALSA_MKDIR_M4
	mkdir -p $(@D)/m4
endef
BLUEZ_ALSA_PRE_CONFIGURE_HOOKS += BLUEZ_ALSA_MKDIR_M4

BLUEZ_ALSA_CONF_OPTS = \
	--enable-aplay \
	--enable-aac \
	--enable-debug \
	--with-alsaplugindir=/usr/lib/alsa-lib \
	--with-alsadatadir=/usr/share/alsa

ifeq ($(BR2_PACKAGE_FDK_AAC),y)
BLUEZ_ALSA_DEPENDENCIES += fdk-aac
BLUEZ_ALSA_CONF_OPTS += --enable-aac
else
BLUEZ_ALSA_CONF_OPTS += --disable-aac
endif

ifeq ($(BR2_PACKAGE_BLUEZ_ALSA_HCITOP),y)
BLUEZ_ALSA_DEPENDENCIES += libbsd ncurses
BLUEZ_ALSA_CONF_OPTS += --enable-hcitop
else
BLUEZ_ALSA_CONF_OPTS += --disable-hcitop
endif

$(eval $(autotools-package))
