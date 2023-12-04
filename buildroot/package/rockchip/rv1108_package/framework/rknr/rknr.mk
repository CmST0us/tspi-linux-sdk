RKNR_SITE = $(TOPDIR)/../framework/rknr
RKNR_SITE_METHOD = local
RKNR_INSTALL_STAGING = YES

# add dependencies
RKNR_DEPENDENCIES =

RKNR_CONFIGURE_DEP_CONFIGS +=

$(eval $(cmake-package))
