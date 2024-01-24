################################################################################
#
# libsrt
#
################################################################################
LIBSRT_VERSION = v1.5.1
LIBSRT_SITE = $(call github,Haivision,srt,$(LIBSRT_VERSION))
LIBSRT_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_HAS_OPENSSL),y)
LIBSRT_DEPENDENCIES += openssl
else
LIBSRT_CONF_OPTS += -DENABLE_ENCRYPTION=OFF
endif

$(eval $(cmake-package))
