STEREO_MSGS_VERSION = 1.11.9
STEREO_MSGS_SITE = $(call github,ros,common_msgs,$(STEREO_MSGS_VERSION))
STEREO_MSGS_SUBDIR = stereo_msgs

STEREO_MSGS_DEPENDENCIES = sensor-msgs

$(eval $(catkin-package))
