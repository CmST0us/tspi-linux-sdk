# Rockchip's MPP(Multimedia Processing Platform)
JPEGDEMO_SITE = $(TOPDIR)/package/rockchip/jpegdemo/src
JPEGDEMO_VERSION = release
JPEGDEMO_SITE_METHOD = local

JPEGDEMO_CONF_DEPENDENCIES += libdrm

$(eval $(cmake-package))
