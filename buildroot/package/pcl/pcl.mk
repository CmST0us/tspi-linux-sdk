###############################################################################
#
# PCL
#
###############################################################################

PCL_VERSION = cc04109d637704e879dbd95ed28781e091bae9a5
PCL_SITE = $(call github,PointCloudLibrary,pcl,$(PCL_VERSION))
PCL_LICENSE = LICENSE.txt
PCL_INSTALL_STAGING = YES

PCL_DEPENDENCIES += boost eigen flann

ifeq ($(BR2_PACKAGE_LIBUSB_COMPAT),y)
PCL_DEPENDENCIES += libusb-compat
endif

ifeq ($(BR2_PACKAGE_LIBPNG),y)
PCL_DEPENDENCIES += libpng
endif

# TODO: Add QT5 depends and support VTK
PCL_CONF_OPTS += -DWITH_QT=OFF -DWITH_VTK=OKK

ifeq ($(BR2_PACKAGE_HAS_LIBGL),)
PCL_CONF_OPTS += -DWITH_OPENGL=OFF
endif

$(eval $(cmake-package))
