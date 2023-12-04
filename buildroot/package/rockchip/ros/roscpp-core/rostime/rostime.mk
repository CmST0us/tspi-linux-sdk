#default to KINETIC
ROSTIME_VERSION = 0.6.11

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSTIME_VERSION = 0.5.8
endif

ROSTIME_SITE = $(call github,ros,roscpp_core,$(ROSTIME_VERSION))
ROSTIME_SUBDIR = rostime
ROSTIME_DEPENDENCIES += boost cpp_common

$(eval $(catkin-package))
