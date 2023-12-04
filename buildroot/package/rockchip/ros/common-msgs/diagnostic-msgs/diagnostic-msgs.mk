DIAGNOSTIC_MSGS_VERSION = 1.11.9
DIAGNOSTIC_MSGS_SITE = $(call github,ros,common_msgs,$(DIAGNOSTIC_MSGS_VERSION))
DIAGNOSTIC_MSGS_SUBDIR = diagnostic_msgs

$(eval $(catkin-package))
