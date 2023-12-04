SENSOR_MSGS_VERSION = 1.11.9
SENSOR_MSGS_SITE = $(call github,ros,common_msgs,$(SENSOR_MSGS_VERSION))
SENSOR_MSGS_SUBDIR = sensor_msgs

SENSOR_MSGS_DEPENDENCIES = geometry-msgs

$(eval $(catkin-package))
