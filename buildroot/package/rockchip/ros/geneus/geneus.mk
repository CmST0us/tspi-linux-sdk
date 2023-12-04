GENEUS_VERSION = 2.2.6
GENEUS_SITE = $(call github,jsk-ros-pkg,geneus,$(GENEUS_VERSION))
GENEUS_INSTALL_STAGING = YES

GENEUS_DEPENDENCIES = genmsg

$(eval $(catkin-package))
