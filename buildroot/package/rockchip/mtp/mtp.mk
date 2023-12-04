MTP_VERSION = 71
MTP_SITE = https://launchpad.net/mtp
MTP_SITE_METHOD = bzr
MTP_INSTALL_STAGING = YES

MTP_DEPENDENCIES = boost dbus-cpp glog

define MTP_ADD_PTHREAD_TO_CMAKE_CXX_FLAGS
	$(SED) 's%-std=c++11%-std=c++11 -lpthread%' $(@D)/CMakeLists.txt
endef
MTP_POST_PATCH_HOOKS += MTP_ADD_PTHREAD_TO_CMAKE_CXX_FLAGS

$(eval $(cmake-package))
