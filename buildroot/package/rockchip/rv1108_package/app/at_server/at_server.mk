AT_SERVER_SITE = $(TOPDIR)/../app/at_server
AT_SERVER_SITE_METHOD = local
AT_SERVER_INSTALL_STAGING = YES

# add dependencies
AT_SERVER_DEPENDENCIES = hal adk messenger

$(eval $(cmake-package))
