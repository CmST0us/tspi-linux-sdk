MINILOGGER_SITE = $(TOPDIR)/../external/minilogger
MINILOGGER_SITE_METHOD = local

MINILOGGER_INSTALL_STAGING = YES

MINILOGGER_DEPENDENCIES = libglib2 libunwind

$(eval $(cmake-package))
