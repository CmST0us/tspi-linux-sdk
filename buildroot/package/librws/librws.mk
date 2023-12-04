################################################################################
#
# librws
#
################################################################################

LIBRWS_VERSION = 901326e962322221a83dfa4b56d02dbb2d276aa8
LIBRWS_SITE = https://github.com/OlehKulykov/librws.git
LIBRWS_SITE_METHOD = git
LIBRWS_LICENSE = MIT
LIBRWS_LICENSE_FILES = LICENSE
LIBRWS_INSTALL_STAGING = YES

LIBRWS_CONF_OPTS = -DRWS_OPT_TESTS=OFF

ifeq ($(BR2_STATIC_LIBS),y)
LIBRWS_CONF_OPTS += -DRWS_OPT_SHARED=OFF
endif

ifeq ($(BR2_SHARED_LIBS),y)
LIBRWS_CONF_OPTS += -DRWS_OPT_STATIC=OFF
endif

$(eval $(cmake-package))
