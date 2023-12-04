HECTOR_MAPPING_VERSION = 0.3.4
HECTOR_MAPPING_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_MAPPING_VERSION))
HECTOR_MAPPING_SUBDIR = hector_mapping

HECTOR_MAPPING_DEPENDENCIES = cmake_modules roscpp nav-msgs visualization-msgs \
			      tf message-filters laser-geometry \
			      tf-conversions message-generation

$(eval $(catkin-package))
