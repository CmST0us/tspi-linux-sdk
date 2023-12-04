# Rockchip's softap demo
# Author : Jacky <jorge.ge@rock-chips.com>

ifeq ($(BR2_PACKAGE_SOFTAP), y)
SOFTAP_SITE = $(TOPDIR)/../external/softapDemo
SOFTAP_SITE_METHOD = local
SOFTAP_INSTALL_STAGING = YES

$(eval $(cmake-package))
endif


