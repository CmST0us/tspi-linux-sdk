HECTOR_TRAJECTORY_SERVER_VERSION = 0.3.4
HECTOR_TRAJECTORY_SERVER_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_TRAJECTORY_SERVER_VERSION))
HECTOR_TRAJECTORY_SERVER_SUBDIR = hector_trajectory_server

HECTOR_TRAJECTORY_SERVER_DEPENDENCIES = roscpp hector-nav-msgs nav-msgs \
					hector-map-tools tf

$(eval $(catkin-package))
