MESSAGE_RUNTIME_VERSION = 0.4.12
MESSAGE_RUNTIME_SITE = $(call github,ros,message_runtime,$(MESSAGE_RUNTIME_VERSION))

MESSAGE_RUNTIME_DEPENDENCIES = gencpp cpp_common roscpp_serialization

$(eval $(catkin-package))
