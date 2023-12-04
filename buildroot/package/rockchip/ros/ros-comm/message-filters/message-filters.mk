#default to KINETIC
MESSAGE_FILTERS_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
MESSAGE_FILTERS_VERSION = 1.11.21
endif

MESSAGE_FILTERS_SITE = $(call github,ros,ros_comm,$(MESSAGE_FILTERS_VERSION))
MESSAGE_FILTERS_SUBDIR = utilities/message_filters

MESSAGE_FILTERS_DEPENDENCIES = boost rosconsole roscpp xmlrpcpp rostest

$(eval $(catkin-package))
