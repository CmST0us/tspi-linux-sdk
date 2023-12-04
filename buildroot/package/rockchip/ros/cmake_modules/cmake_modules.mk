CMAKE_MODULES_VERSION = 0.3.3
CMAKE_MODULES_SITE = $(call github,ros,cmake_modules,$(CMAKE_MODULES_VERSION))
CMAKE_MODULES_INSTALL_STAGING = YES

${eval ${catkin-package}}
