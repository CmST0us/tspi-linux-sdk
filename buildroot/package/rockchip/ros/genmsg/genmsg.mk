GENMSG_VERSION = 0.5.8
GENMSG_SITE = $(call github,ros,genmsg,$(GENMSG_VERSION))

$(eval $(catkin-package))
