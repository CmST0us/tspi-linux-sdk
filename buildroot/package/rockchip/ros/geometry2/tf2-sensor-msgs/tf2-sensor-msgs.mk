TF2_SENSOR_MSGS_VERSION = 0.5.17

TF2_SENSOR_MSGS_SITE = $(call github,ros,geometry2,$(TF2_SENSOR_MSGS_VERSION))
TF2_SENSOR_MSGS_SUBDIR = tf2_sensor_msgs
TF2_SENSOR_MSGS_DEPENDENCIES = sensor-msgs cmake_modules tf2-ros boost

$(eval $(catkin-package))
