HECTOR_MARKER_DRAWING_VERSION = 0.3.4
HECTOR_MARKER_DRAWING_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_MARKER_DRAWING_VERSION))
HECTOR_MARKER_DRAWING_SUBDIR = hector_marker_drawing

HECTOR_MARKER_DRAWING_DEPENDENCIES = eigen cmake_modules roscpp visualization-msgs

$(eval $(catkin-package))
