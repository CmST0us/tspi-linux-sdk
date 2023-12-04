TF_VERSION = 1.11.9

TF_SITE = $(call github,ros,geometry,$(TF_VERSION))
TF_SUBDIR = tf

TF_DEPENDENCIES = angles geometry-msgs message-filters message-generation \
		  rosconsole roscpp rostime sensor-msgs std-msgs tf2-ros

$(eval $(catkin-package))
