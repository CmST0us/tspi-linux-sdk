# Rockchip's alexa c++ sdk
# Author : Nyx Zheng <zyh@rock-chips.com>

#ifeq ($(BR2_PACKAGE_LOCAL_PLAYER),y)
LOCALPLAYER_SITE = $(TOPDIR)/../external/LocalPlayer
LOCALPLAYER_SITE_METHOD = local
LOCALPLAYER_INSTALL_STAGING = YES
LOCALPLAYER_DEPENDENCIES = gst1-plugins-base gst1-plugins-ugly
LOCALPLAYER_CONF_OPTS +=\
						   CMAKE_CURRENT_SOURCE_DIR= source \
						   -DLIBRARY_OUTPUT_PATH=$(TOPDIR)/$(BR2_ROOTFS_OVERLAY)/usr/lib \
						   -DEXECUTABLE_OUTPUT_PATH=$(TOPDIR)/$(BR2_ROOTFS_OVERLAY)/usr/bin \
						   -DCMAKE_BUILD_TYPE=DEBUG \
						   -DGSTREAMER_MEDIA_PLAYER=ON

$(eval $(cmake-package))

#endif


