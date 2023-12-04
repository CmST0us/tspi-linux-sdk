#default to KINETIC
TOPIC_TOOLS_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
TOPIC_TOOLS_VERSION = 1.11.21
endif

TOPIC_TOOLS_SITE = $(call github,ros,ros_comm,$(TOPIC_TOOLS_VERSION))
TOPIC_TOOLS_SUBDIR = tools/topic_tools

TOPIC_TOOLS_DEPENDENCIES = cpp_common message-generation rosconsole roscpp rostime std-msgs \
						   xmlrpcpp rostest

$(eval $(catkin-package))
