CAMERA_CALIBRATION_PARSERS_VERSION = 1.11.11
CAMERA_CALIBRATION_PARSERS_SITE = $(call github,ros-perception,image_common,$(CAMERA_CALIBRATION_PARSERS_VERSION))
CAMERA_CALIBRATION_PARSERS_SUBDIR = camera_calibration_parsers

CAMERA_CALIBRATION_PARSERS_DEPENDENCIES = boost yaml-cpp sensor-msgs rosconsole roscpp roscpp_serialization

$(eval $(catkin-package))
