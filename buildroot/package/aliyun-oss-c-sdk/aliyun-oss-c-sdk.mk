################################################################################
#
# aliyun-oss-c-sdk
#
################################################################################

ALIYUN_OSS_C_SDK_VERSION = 3.7.1
ALIYUN_OSS_C_SDK_SITE = $(call github,aliyun,aliyun-oss-c-sdk,$(ALIYUN_OSS_C_SDK_VERSION))
#ALIYUN_OSS_C_SDK_SITE = $(TOPDIR)/../external/oss_sdk
#ALIYUN_OSS_C_SDK_SITE_METHOD = local
ALIYUN_OSS_C_SDK_LICENSE = MIT
ALIYUN_OSS_C_SDK_LICENSE_FILES = COPYING
ALIYUN_OSS_C_SDK_INSTALL_STAGING = YES
ALIYUN_OSS_C_SDK_DEPENDENCIES = host-pkgconf apr apr-util libcurl mxml

ALIYUN_OSS_C_SDK_CONF_OPTS += -DAPR_CONFIG_PATH=$(STAGING_DIR)/usr/bin
$(eval $(cmake-package))
