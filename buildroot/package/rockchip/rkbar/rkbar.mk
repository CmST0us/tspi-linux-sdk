RKBAR_SITE = $(TOPDIR)/../external/rkbar
RKBAR_SITE_METHOD = local
RKBAR_INSTALL_STAGING = YES

# add dependencies
RKBAR_DEPENDENCIES =

$(eval $(cmake-package))

