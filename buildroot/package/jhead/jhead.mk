################################################################################
#
# jhead
#
################################################################################

JHEAD_VERSION = 3.00
JHEAD_SITE = http://www.sentex.net/~mwandel/jhead
JHEAD_LICENSE = Public Domain
JHEAD_LICENSE_FILES = readme.txt

ifeq ($(BR2_PACKAGE_LIBJHEAD),y)
JHEAD_MAKE_ENV += CFLAGS+="-fPIC" LIBJHEAD=y

define GEN_LIBJHEAD_C
	cp -a $(@D)/jhead.c $(@D)/libjhead.c
	sed -i '/^int main/,$$d' $(@D)/libjhead.c
endef
JHEAD_PRE_PATCH_HOOKS = GEN_LIBJHEAD_C
endif

define JHEAD_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(JHEAD_MAKE_ENV) -C $(@D)
endef

define JHEAD_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/jhead $(TARGET_DIR)/usr/bin/jhead
	$(INSTALL) -m 0755 -D $(@D)/libjhead.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -m 0755 -D $(@D)/libjhead.so $(STAGING_DIR)/usr/lib/
	$(INSTALL) -m 0755 -D $(@D)/jhead.h $(STAGING_DIR)/usr/include/
endef

$(eval $(generic-package))
