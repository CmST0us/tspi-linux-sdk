GSTREAMER1_IEP_SITE = "https://github.com/rockchip-linux/gstreamer1-iep"
GSTREAMER1_IEP_VERSION = master
GSTREAMER1_IEP_SITE_METHOD = git
GSTREAMER1_IEP_GIT_SUBMODULES = YES

GSTREAMER1_IEP_LICENSE_FILES = COPYING
GSTREAMER1_IEP_LICENSE = GPLv2.1+
GSTREAMER1_IEP_AUTORECONF = YES
GSTREAMER1_IEP_GETTEXTIZE = YES
GSTREAMER1_IEP_DEPENDENCIES = rockchip-mpp gst1-plugins-base

GSTREAMER1_IEP_CONF_OPTS = \
	--disable-valgrind \
	--disable-examples \
	--disable-kms

$(eval $(autotools-package))
