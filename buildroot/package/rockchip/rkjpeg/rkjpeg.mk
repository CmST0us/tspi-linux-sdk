# Rockchip's MPP(Multimedia Processing Platform)
RKJPEG_SITE = $(TOPDIR)/package/rockchip/rkjpeg/src
RKJPEG_VERSION = release
RKJPEG_SITE_METHOD = local

RKJPEG_CONF_DEPENDENCIES += libdrm

$(eval $(cmake-package))
