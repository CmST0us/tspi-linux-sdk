HECTOR_SLAM_VERSION = 0.3.4
HECTOR_SLAM_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_SLAM_VERSION))
HECTOR_SLAM_SUBDIR = hector_slam

$(eval $(catkin-package))
