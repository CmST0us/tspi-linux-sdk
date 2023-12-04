MESSENGER_SITE = $(TOPDIR)/../framework/messenger
MESSENGER_SITE_METHOD = local
MESSENGER_INSTALL_STAGING = YES

# add dependencies
MESSENGER_DEPENDENCIES = nanomsg cjson adk

$(eval $(cmake-package))
