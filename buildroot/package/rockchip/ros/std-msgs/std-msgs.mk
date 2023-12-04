STD_MSGS_VERSION = 0.5.10
STD_MSGS_SITE = $(call github,ros,std_msgs,$(STD_MSGS_VERSION))

STD_MSGS_DEPENDENCIES = message-generation message-runtime

ifeq ($(BR2_PACKAGE_ROS_KINETIC),y)
STD_MSGS_DEPENDENCIES += host-python-pyyaml
endif

$(eval $(catkin-package))

