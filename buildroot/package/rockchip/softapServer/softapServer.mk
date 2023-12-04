# Rockchip's softap server demo
# Author : Jacky <jorge.ge@rock-chips.com>

ifeq ($(BR2_PACKAGE_SOFTAPSERVER), y)
SOFTAPSERVER_SITE = $(TOPDIR)/../external/softapServer
SOFTAPSERVER_SITE_METHOD = local
SOFTAPSERVER_INSTALL_STAGING = YES

$(eval $(cmake-package))
endif


