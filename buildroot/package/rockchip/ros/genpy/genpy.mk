#default to KINETIC
GENPY_VERSION = 0.6.7

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
GENPY_VERSION = 0.5.10
endif

GENPY_SITE = $(call github,ros,genpy,$(GENPY_VERSION))
GENPY_INSTALL_STAGING = YES
GENPY_DEPENDENCIES = genmsg

$(eval $(catkin-package))
