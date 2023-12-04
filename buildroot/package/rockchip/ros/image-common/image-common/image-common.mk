IMAGE_COMMON_VERSION = 1.11.11
IMAGE_COMMON_SITE = $(call github,ros-perception,image_common,$(IMAGE_COMMON_VERSION))
IMAGE_COMMON_SUBDIR = image_common

$(eval $(catkin-package))
