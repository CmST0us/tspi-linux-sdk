#T###############################################################################
#
# alsa-utils
#
################################################################################

ALSA_PLUGINS_VERSION = 1.1.5
ALSA_PLUGINS_SOURCE = alsa-plugins-$(ALSA_PLUGINS_VERSION).tar.bz2
ALSA_PLUGINS_SITE = ftp://ftp.alsa-project.org/pub/plugins
ALSA_PLUGINS_LICENSE = GPLv2
ALSA_PLUGINS_LICENSE_FILES = COPYING
ALSA_PLUGINS_INSTALL_STAGING = YES
ALSA_PLUGINS_DEPENDENCIES = host-pkgconf alsa-lib

ifeq ($(BR2_PACKAGE_FFMPEG),y)
ALSA_PLUGINS_DEPENDENCIES += ffmpeg
ALSA_PLUGINS_CONF_OPTS += --enable-avcodec
else
ALSA_PLUGINS_CONF_OPTS += --disable-avcodec
endif

ifeq ($(BR2_PACKAGE_PULSEAUDIO),y)
ALSA_PLUGINS_DEPENDENCIES += pulseaudio
ALSA_PLUGINS_CONF_OPTS += --enable-pulseaudio

define ALSA_PLUGINS_DEFAULT_PULSEAUDIO
	cd $(TARGET_DIR)/usr/share/alsa/alsa.conf.d && \
		mv 99-pulseaudio-default.conf.example 99-pulseaudio-default.conf
endef
ALSA_PLUGINS_POST_INSTALL_TARGET_HOOKS += ALSA_PLUGINS_DEFAULT_PULSEAUDIO

else
ALSA_PLUGINS_CONF_OPTS += --disable-pulseaudio
endif

ifeq ($(BR2_PACKAGE_LIBSAMPLERATE),y)
ALSA_PLUGINS_DEPENDENCIES += libsamplerate
ALSA_PLUGINS_CONF_OPTS += --enable-samplerate
else
ALSA_PLUGINS_CONF_OPTS += --disable-samplerate
endif

ifeq ($(BR2_PACKAGE_SPEEX),y)
ALSA_PLUGINS_DEPENDENCIES += speex
ALSA_PLUGINS_CONF_OPTS += --with-speex=lib
else
ALSA_PLUGINS_CONF_OPTS += --with-speex=builtin
endif

$(eval $(autotools-package))
