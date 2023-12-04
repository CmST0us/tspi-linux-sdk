GENNODEJS_VERSION = 2.0.1
GENNODEJS_SITE = $(call github,RethinkRobotics-opensource,gennodejs,$(GENNODEJS_VERSION))
GENNODEJS_INSTALL_STAGING = YES

GENNODEJS_DEPENDENCIES = genmsg

$(eval $(catkin-package))
