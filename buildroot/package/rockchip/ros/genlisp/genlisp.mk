GENLISP_VERSION = 0.4.15
GENLISP_SITE = $(call github,ros,genlisp,$(GENLISP_VERSION))

GENLISP_DEPENDENCIES = genmsg

$(eval $(catkin-package))
