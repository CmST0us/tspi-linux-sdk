ROSGRAPH_MSGS_VERSION = 1.11.2
ROSGRAPH_MSGS_SITE = $(call github,ros,ros_comm_msgs,$(ROSGRAPH_MSGS_VERSION))
ROSGRAPH_MSGS_SUBDIR = rosgraph_msgs

ROSGRAPH_MSGS_DEPENDENCIES = message-generation std-msgs

$(eval $(catkin-package))
