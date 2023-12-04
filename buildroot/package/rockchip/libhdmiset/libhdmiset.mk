# Rockchip's MPP(Multimedia Processing Platform)

#LIBHDMISET_SITE = $(TOPDIR)/../external/libhdmiset
ifeq ($(BR2_PACKAGE_LIBHDMISET),y)

LIBHDMISET_VERSION = master
LIBHDMISET_SITE = https://github.com/qiuen/drmhdmiset.git
LIBHDMISET_SITE_METHOD = git
LIBHDMISET_DEPENDENCIES += libhdmiset
LIBHDMISET_INSTALL_STAGING = YES

endif
$(eval $(cmake-package))
