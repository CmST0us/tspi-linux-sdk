HECTOR_IMU_TOOLS_VERSION = 0.3.4
HECTOR_IMU_TOOLS_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_IMU_TOOLS_VERSION))
HECTOR_IMU_TOOLS_SUBDIR = hector_imu_tools

HECTOR_IMU_TOOLS_DEPENDENCIES = roscpp sensor-msgs geometry-msgs nav-msgs tf

$(eval $(catkin-package))
