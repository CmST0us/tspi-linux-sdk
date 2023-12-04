LASER_GEOMETRY_VERSION = 1.6.4
LASER_GEOMETRY_SITE = $(call github,ros-perception,laser_geometry,$(LASER_GEOMETRY_VERSION))

LASER_GEOMETRY_DEPENDENCIES = eigen boost angles roscpp sensor-msgs tf tf2

${eval ${catkin-package}}
