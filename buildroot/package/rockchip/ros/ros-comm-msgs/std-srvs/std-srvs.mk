STD_SRVS_VERSION = 1.11.2
STD_SRVS_SITE = $(call github,ros,ros_comm_msgs,$(STD_SRVS_VERSION))
STD_SRVS_SUBDIR = std_srvs

$(eval $(catkin-package))
