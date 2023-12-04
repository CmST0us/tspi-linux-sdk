ROSPACK_VERSION = 2.4.4
ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSPACK_VERSION = 2.2.8
endif
ROSPACK_SITE = $(call github,ros,rospack,$(ROSPACK_VERSION))

ROSPACK_DEPENDENCIES = boost tinyxml cmake_modules python

${eval ${catkin-package}}
