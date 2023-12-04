#default to KINETIC
ROSCPP_CORE_VERSION = 0.6.11

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSCPP_CORE_VERSION = 0.5.8
endif

ROSCPP_CORE_SITE = $(call github,ros,roscpp_core,$(ROSCPP_CORE_VERSION))
ROSCPP_CORE_SUBDIR = roscpp_core
ROSCPP_CORE_DEPENDENCIES += cpp_common rostime roscpp_traits roscpp_serialization

$(eval $(catkin-package))
