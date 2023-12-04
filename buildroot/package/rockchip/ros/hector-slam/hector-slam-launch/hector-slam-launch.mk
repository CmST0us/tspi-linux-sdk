HECTOR_SLAM_LAUNCH_VERSION = 0.3.4
HECTOR_SLAM_LAUNCH_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_SLAM_LAUNCH_VERSION))
HECTOR_SLAM_LAUNCH_SUBDIR = hector_slam_launch

$(eval $(catkin-package))
