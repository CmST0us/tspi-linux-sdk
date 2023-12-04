TF2_MSGS_VERSION = 0.5.17

TF2_MSGS_SITE = $(call github,ros,geometry2,$(TF2_MSGS_VERSION))
TF2_MSGS_SUBDIR = tf2_msgs

TF2_MSGS_DEPENDENCIES = message-generation geometry-msgs actionlib-msgs boost

$(eval $(catkin-package))
