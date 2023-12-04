COMMON_MSGS_VERSION = 1.11.9
COMMON_MSGS_SITE = $(call github,ros,common_msgs,$(COMMON_MSGS_VERSION))
COMMON_MSGS_SUBDIR = common_msgs

$(eval $(catkin-package))
