#default to KINETIC
ROSCONSOLE_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSCONSOLE_VERSION = 1.11.21
endif

ROSCONSOLE_SITE = $(call github,ros,ros_comm,$(ROSCONSOLE_VERSION))
ROSCONSOLE_SUBDIR = tools/rosconsole

ROSCONSOLE_DEPENDENCIES = \
	cpp_common rostime rosunit boost

$(eval $(catkin-package))
