#default to KINETIC
ROSCPP_TRAITS_VERSION = 0.6.11

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSCPP_TRAITS_VERSION = 0.5.8
endif

ROSCPP_TRAITS_SITE = $(call github,ros,roscpp_core,$(ROSCPP_TRAITS_VERSION))
ROSCPP_TRAITS_SUBDIR = roscpp_traits

ROSCPP_TRAITS_DEPENDENCIES = cpp_common rostime

$(eval $(catkin-package))
