ANGLES_VERSION = 1.9.11

ANGLES_SITE = $(call github,ros,angles,$(ANGLES_VERSION))
ANGLES_SUBDIR = angles

$(eval $(catkin-package))
