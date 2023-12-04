#COMMON_MSGS_VERSION = 1.11.9
#COMMON_MSGS_SITE = $(call github,ros,common_msgs,$(COMMON_MSGS_VERSION))

include $(sort $(wildcard package/rockchip/ros/common-msgs/*/*.mk))
