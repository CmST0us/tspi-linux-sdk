GENCPP_VERSION = 0.5.5
GENCPP_SITE = $(call github,ros,gencpp,$(GENCPP_VERSION))
GENCPP_INSTALL_STAGING = YES

GENCPP_DEPENDENCIES = genmsg

$(eval $(catkin-package))
