RV1108_LIBPNG_SITE = $(TOPDIR)/../external/libpng
RV1108_LIBPNG_SITE_METHOD = local
RV1108_LIBPNG_INSTALL_STAGING = YES

RV1108_LIBPNG_DEPENDENCIES = zlib

RV1108_LIBPNG_CONF_OPTS = \
	--host=arm-linux \
	 --with-pic

INCLUDE_PATH=$(STAGING_DIR)/usr/include/
define RV1108_LIBPNG_CONFIGURE_CMDS
    cd $(@D); $(TARGET_MAKE_ENV) ./configure $(RV1108_LIBPNG_CONF_OPTS) CFLAGS="$(CFLAGS) -I $(INCLUDE_PATH)"
endef

define RV1108_LIBPNG_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define RV1108_LIBPNG_INSTALL_STAGING_CMDS
    $(INSTALL) -D $(@D)/*.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -D $(@D)/.libs/*.so* $(STAGING_DIR)/usr/lib/
endef

#define RV1108_LIBPNG_INSTALL_TARGET_CMDS
#    $(INSTALL) -D $(@D)/*.h $(TARGET_DIR)/usr/include/
#    $(INSTALL) -D $(@D)/.libs/*.so* $(TARGET_DIR)/usr/lib/
#endef

$(eval $(generic-package))
