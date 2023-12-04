#default to KINETIC
ROSLANG_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSLANG_VERSION = 1.11.14
endif

ROSLANG_SITE = $(call github,ros,ros,$(ROSLANG_VERSION))
ROSLANG_SUBDIR = core/roslang

$(eval $(catkin-package))
