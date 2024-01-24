################################################################################
#
# rknn demo
#
################################################################################

RKNN_DEMO_VERSION = 1.0.0
RKNN_DEMO_SITE = $(TOPDIR)/../external/rknn_demo
RKNN_DEMO_SITE_METHOD = local
RKNN_DEMO_DEPENDENCIES = jpeg libpng libv4l rockchip-rga minigui camera_engine_rkisp libdrm rockchip-mpp uvc_app

ifeq ($(BR2_PACKAGE_RK1808),y)
	RKNN_DEMO_CONF_OPTS += -DNEED_RKNNAPI=0
	RKNN_DEMO_DEPENDENCIES += rknpu
endif

ifeq ($(BR2_PACKAGE_RK3399PRO),y)
	RKNN_DEMO_CONF_OPTS += -DNEED_RKNNAPI=1
define RKNN_DEMO_BUILD_CMDS
		$(INSTALL) -D -m 0644 $(@D)/rknn/rknn_api/librknn_api.so $(TARGET_DIR)/usr/lib
		$(INSTALL) -D -m 0644 $(@D)/rknn/rknn_api/librknn_api.so $(STAGING_DIR)/usr/lib
		$(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) $($(PKG)_MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_BUILDDIR)
endef
endif

RKNN_DEMO_USER_CONF_OPTS =

ifeq ($(BR2_PACKAGE_RKNN_DEMO_ENABLE_JOINT),y)
	RKNN_DEMO_USER_CONF_OPTS += -DENABLE_JOINT=1
endif

ifeq ($(BR2_PACKAGE_RKNN_DEMO_ENABLE_FRG),y)
	RKNN_DEMO_USER_CONF_OPTS += -DENABLE_FRG=1
endif

ifeq ($(RKNN_DEMO_USER_CONF_OPTS),)
	BR2_PACKAGE_RKNN_DEMO_ENABLE_SSD=y
	RKNN_DEMO_CONF_OPTS += -DENABLE_SSD=1
else
	RKNN_DEMO_CONF_OPTS += $(RKNN_DEMO_USER_CONF_OPTS)
endif

ifeq ($(BR2_PACKAGE_RK1808),y)
RKNN_DEMO_MINIGUI_CFG=minigui/MiniGUI-1280x720.cfg
endif

ifeq ($(BR2_PACKAGE_RK3399PRO),y)
RKNN_DEMO_MINIGUI_CFG=minigui/MiniGUI-2048x1536.cfg
endif

RKNN_DEMO_CONF_OPTS += -DMINIGUI_CFG=$(RKNN_DEMO_MINIGUI_CFG)

ifeq ($(BR2_PACKAGE_RKNN_DEMO_ENABLE_JOINT),y)
RKNN_MODEL_RESOURCE_FILES = rknn/joint/cpm.rknn
endif

ifeq ($(BR2_PACKAGE_RKNN_DEMO_ENABLE_FRG),y)
RKNN_MODEL_RESOURCE_FILES = rknn/frg/frgsdk_rk1808/model/align.rknn \
							rknn/frg/frgsdk_rk1808/model/detect.rknn \
							rknn/frg/frgsdk_rk1808/model/recognize.rknn
endif

ifeq ($(BR2_PACKAGE_RKNN_DEMO_ENABLE_SSD),y)
ifeq ($(BR2_PACKAGE_RK1808),y)
RKNN_MODEL_RESOURCE_FILES = rknn/ssd/ssd_1808/ssd_inception_v2.rknn \
							rknn/ssd/ssd_1808/coco_labels_list.txt \
							rknn/ssd/ssd_1808/box_priors.txt
else
RKNN_MODEL_RESOURCE_FILES = rknn/ssd/ssd_3399pro/mobilenet_ssd.rknn \
							rknn/ssd/ssd_3399pro/coco_labels_list.txt \
							rknn/ssd/ssd_3399pro/box_priors.txt
endif
endif

RKNN_DEMO_CONF_OPTS += -DMODEL_RESOURCE_FILES="$(RKNN_MODEL_RESOURCE_FILES)"

$(eval $(cmake-package))
