################################################################################
#
# rockit project
#
################################################################################

ROCKIT_SITE = $(TOPDIR)/../external/rockit

ROCKIT_SITE_METHOD = local

ROCKIT_INSTALL_STAGING = YES

$(eval $(cmake-package))
