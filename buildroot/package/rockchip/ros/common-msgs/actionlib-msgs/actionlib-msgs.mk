ACTIONLIB_MSGS_VERSION = 1.11.9
ACTIONLIB_MSGS_SITE = $(call github,ros,common_msgs,$(ACTIONLIB_MSGS_VERSION))
ACTIONLIB_MSGS_SUBDIR = actionlib_msgs

ACTIONLIB_MSGS_DEPENDENCIES = \
	cpp_common roscpp_serialization \
	message-generation message-runtime std-msgs

$(eval $(catkin-package))
