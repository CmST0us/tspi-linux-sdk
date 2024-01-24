################################################################################
#
# alsa-plugins
#
################################################################################

ALSA_PLUGINS_VERSION = 1.2.6
ALSA_PLUGINS_SOURCE = alsa-plugins-$(ALSA_PLUGINS_VERSION).tar.bz2
ALSA_PLUGINS_SITE = https://www.alsa-project.org/files/pub/plugins
ALSA_PLUGINS_LICENSE = LGPL-2.1+
ALSA_PLUGINS_LICENSE_FILES = COPYING
ALSA_PLUGINS_DEPENDENCIES = host-pkgconf alsa-lib

ALSA_PLUGINS_CONF_OPTS = \
	--disable-jack \
	--disable-usbstream \
	--disable-maemo-plugin \
	--disable-maemo-resource-manager

ifeq ($(BR2_PACKAGE_FFMPEG),y)
ALSA_PLUGINS_DEPENDENCIES += ffmpeg
ALSA_PLUGINS_CONF_OPTS += --enable-libav
else
ALSA_PLUGINS_CONF_OPTS += --disable-libav
endif

ifeq ($(BR2_PACKAGE_PULSEAUDIO),y)
ALSA_PLUGINS_DEPENDENCIES += pulseaudio
ALSA_PLUGINS_CONF_OPTS += --enable-pulseaudio

define ALSA_PLUGINS_DEFAULT_PULSEAUDIO
	cd $(TARGET_DIR)/etc/alsa/conf.d && \
		mv 99-pulseaudio-default.conf.example 99-pulseaudio-default.conf
endef
ALSA_PLUGINS_POST_INSTALL_TARGET_HOOKS += ALSA_PLUGINS_DEFAULT_PULSEAUDIO

else
ALSA_PLUGINS_CONF_OPTS += --disable-pulseaudio
endif

ifeq ($(BR2_PACKAGE_SPEEX),y)
ALSA_PLUGINS_DEPENDENCIES += speex
ALSA_PLUGINS_CONF_OPTS += --with-speex=lib
else
ALSA_PLUGINS_CONF_OPTS += --with-speex=builtin
endif

ifeq ($(BR2_PACKAGE_LIBSAMPLERATE),y)
ALSA_PLUGINS_CONF_OPTS += --enable-samplerate
ALSA_PLUGINS_DEPENDENCIES += libsamplerate
ALSA_PLUGINS_LICENSE += , GPL-2.0+ (samplerate plugin)
ALSA_PLUGINS_LICENSE_FILES += COPYING.GPL
else
ALSA_PLUGINS_CONF_OPTS += --disable-samplerate
endif

$(eval $(autotools-package))
