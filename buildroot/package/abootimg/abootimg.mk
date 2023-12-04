################################################################################
#
# abootimg
#
################################################################################

ABOOTIMG_VERSION = 7e127fee6a3981f6b0a50ce9910267cd501e09d4
ABOOTIMG_SITE = https://github.com/ggrandou/abootimg.git
ABOOTIMG_SITE_METHOD = git
ABOOTIMG_LICENSE = GPL-2.0
ABOOTIMG_LICENSE_FILES = LICENSE

ABOOTIMG_DEPENDENCIES = util-linux

define ABOOTIMG_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define ABOOTIMG_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/abootimg $(TARGET_DIR)/usr/bin/abootimg
	$(INSTALL) -D -m 755 $(@D)/abootimg-pack-initrd $(TARGET_DIR)/usr/bin/abootimg-pack-initrd
	$(INSTALL) -D -m 755 $(@D)/abootimg-unpack-initrd $(TARGET_DIR)/usr/bin/abootimg-unpack-initrd
endef

$(eval $(generic-package))
