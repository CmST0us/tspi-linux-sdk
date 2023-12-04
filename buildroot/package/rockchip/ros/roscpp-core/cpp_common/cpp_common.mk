#default to KINETIC
CPP_COMMON_VERSION = 0.6.11

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
CPP_COMMON_VERSION = 0.5.8
endif

CPP_COMMON_SITE = $(call github,ros,roscpp_core,$(CPP_COMMON_VERSION))
CPP_COMMON_SUBDIR = cpp_common
CPP_COMMON_DEPENDENCIES += console-bridge

$(eval $(catkin-package))
