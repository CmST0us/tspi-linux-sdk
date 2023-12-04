RK_SENSORS_VERSION = 1.0.0
RK_SENSORS_SITE_METHOD = local
RK_SENSORS_SITE = $(TOPDIR)/../external/ros_sensors

RK_SENSORS_DEPENDENCIES = roscpp rospy std-msgs genmsg

$(eval $(catkin-package))
